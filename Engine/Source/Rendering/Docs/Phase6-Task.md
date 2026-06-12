# Phase 6 Task: Material System + Shader Assets

## Objective
Implement a complete Material and Shader Asset system with PSO caching, enabling C# layer to work with materials without knowing about pipeline states or backend details.

## Working Directory
`E:\Neverness\Engine\Source\Experiments`

## Critical Constraints
- NEVER use `using namespace Diligent;` - always `::Diligent::` fully qualified
- `using namespace NN::Runtime::Core;` and `using namespace NN::Runtime::Render;` OK in .cpp
- C++20, MSVC, Windows, ASCII only in source files
- Follow existing code style in NNRuntimeRender interfaces
- NNRuntimeRenderAssets has NO Diligent dependency - uses interfaces only
- All Diligent-specific work stays in NNRuntimeDiligent

## Architecture

```
NNRuntimeRenderAssets (NEW - Asset Layer)
├── Material/
│   ├── NNMaterial.h              ← C# visible material interface
│   ├── NNMaterialInstance.h      ← Material instances with overrides
│   └── NNMaterialParam.h         ← Material parameter storage
├── Shader/
│   ├── NNShaderAsset.h           ← Shader asset (loaded from file/memory)
│   ├── NNShaderVariant.h         ← Shader permutation/variant
│   └── NNShaderSource.h          ← Shader source code management
└── Cache/
    ├── NNPipelineCache.h         ← PSO cache (internal)
    └── NNAssetRegistry.h         ← Asset handle registry

Dependencies:
    NNRuntimeRenderAssets → NNRuntimeRender (interfaces)
    NNRuntimeRenderAssets → NNRuntimeCore (Handle, NNRef)
    NNRuntimeRenderAssets → NNCore (HCoreTypes.h, Math)
```

## Existing Interfaces (DO NOT modify)
- `NNRuntimeRender/Pipeline/INNShader.h` — Shader interface
- `NNRuntimeRender/Pipeline/INNPipelineState.h` — PSO interface
- `NNRuntimeRender/Resources/INNTexture.h` — Texture interface
- `NNRuntimeRender/Resources/INNBuffer.h` — Buffer interface
- `NNRuntimeRender/Command/INNCommandList.h` — Command list
- `NNRuntimeRender/Device/INNRenderDevice.h` — Device interface
- `NNRuntimeRender/RenderTarget/INNRenderTarget.h` — Render target

## Tasks

### Task 1: NNMaterialParam — Parameter Storage
Create `NNRuntimeRenderAssets/Material/NNMaterialParam.h`

```cpp
enum class NNMaterialParamType : uint8_t { Float, Int, Vector4, Texture, Sampler };

struct NNMaterialParam {
    std::string Name;
    NNMaterialParamType Type;
    union { float Float; int Int; float Vec4[4]; };
    NNRef<INNTexture> Texture;
    NNRef<INNSampler> Sampler;
};
```

### Task 2: NNMaterial — Material Interface
Create `NNRuntimeRenderAssets/Material/NNMaterial.h`

```cpp
class NNMaterial : public INNObject {
public:
    void SetFloat(const char* name, float value);
    void SetInt(const char* name, int value);
    void SetVector4(const char* name, float x, float y, float z, float w);
    void SetTexture(const char* name, NNRef<INNTexture> tex);
    void SetSampler(const char* name, NNRef<INNSampler> sampler);
    void SetShader(NNRef<INNShader> shader);

    // Apply material to command list (handles PSO + binding internally)
    void Apply(INNCommandList* cmd, NNPipelineCache* cache,
               const NNVertexLayout& vertexLayout,
               NNPixelFormat rtvFormat, NNPixelFormat dsvFormat);

private:
    NNRef<INNShader> m_VS;
    NNRef<INNShader> m_PS;
    std::vector<NNMaterialParam> m_Params;
    NNRef<INNPipelineState> m_CachedPSO;
};
```

### Task 3: NNMaterialInstance — Material Instances
Create `NNRuntimeRenderAssets/Material/NNMaterialInstance.h`

```cpp
class NNMaterialInstance : public INNObject {
public:
    NNMaterialInstance(NNRef<NNMaterial> base);

    void SetFloat(const char* name, float value);
    void SetTexture(const char* name, NNRef<INNTexture> tex);
    // ... override params without modifying base material

    void Apply(INNCommandList* cmd, NNPipelineCache* cache,
               const NNVertexLayout& vertexLayout,
               NNPixelFormat rtvFormat, NNPixelFormat dsvFormat);

private:
    NNRef<NNMaterial> m_Base;
    std::vector<NNMaterialParam> m_Overrides;
};
```

### Task 4: NNShaderAsset — Shader Asset
Create `NNRuntimeRenderAssets/Shader/NNShaderAsset.h`

```cpp
class NNShaderAsset : public INNObject {
public:
    // Load from file
    static NNRef<NNShaderAsset> LoadFromFile(const char* path, NNShaderStage stage);

    // Load from memory
    static NNRef<NNShaderAsset> LoadFromMemory(const char* source, uint32_t length,
                                                 NNShaderStage stage, const char* name);

    // Create shader for specific backend
    NNRef<INNShader> CreateShader(INNRenderDevice* device);

    // Get source info
    NNShaderStage GetStage() const;
    const char* GetSource() const;
    const char* GetName() const;

private:
    std::string m_Source;
    std::string m_Name;
    NNShaderStage m_Stage;
};
```

### Task 5: NNShaderVariant — Shader Permutations
Create `NNRuntimeRenderAssets/Shader/NNShaderVariant.h`

```cpp
struct NNShaderVariantKey {
    uint32_t FeatureMask;
    uint32_t MaterialFlags;
    // Hash for quick comparison
    uint64_t Hash() const;
};

class NNShaderVariant : public INNObject {
public:
    NNShaderVariant(NNRef<NNShaderAsset> asset, const NNShaderVariantKey& key);

    // Get compiled shader (compiles on first access)
    NNRef<INNShader> GetShader(INNRenderDevice* device);

    const NNShaderVariantKey& GetKey() const;

private:
    NNRef<NNShaderAsset> m_Asset;
    NNShaderVariantKey m_Key;
    NNRef<INNShader> m_CachedShader;
};
```

### Task 6: NNPipelineCache — PSO Cache
Create `NNRuntimeRenderAssets/Cache/NNPipelineCache.h`

```cpp
struct NNPipelineKey {
    INNShader* VS;
    INNShader* PS;
    NNVertexLayout VertexLayout;
    NNRasterizerState Rasterizer;
    NNBlendState Blend;
    NNDepthStencilState DepthStencil;
    NNPixelFormat RTVFormat;
    NNPixelFormat DSVFormat;
    uint32_t SampleCount;
    uint64_t Hash() const;
};

class NNPipelineCache : public INNObject {
public:
    NNPipelineCache(INNRenderDevice* device);

    // Get or create PSO
    NNRef<INNPipelineState> GetOrCreate(const NNPipelineKey& key);

    // Clear cache
    void Clear();

    // Stats
    size_t GetCount() const;

private:
    INNRenderDevice* m_Device;
    std::unordered_map<uint64_t, NNRef<INNPipelineState>> m_Cache;
};
```

### Task 7: NNAssetRegistry — Asset Handle Registry
Create `NNRuntimeRenderAssets/Cache/NNAssetRegistry.h`

```cpp
class NNAssetRegistry : public INNObject {
public:
    // Register asset, returns handle
    NNHandle RegisterShaderAsset(NNRef<NNShaderAsset> asset);
    NNHandle RegisterMaterial(NNRef<NNMaterial> material);
    NNHandle RegisterMaterialInstance(NNRef<NNMaterialInstance> instance);

    // Get asset by handle
    NNShaderAsset* GetShaderAsset(NNHandle handle);
    NNMaterial* GetMaterial(NNHandle handle);
    NNMaterialInstance* GetMaterialInstance(NNHandle handle);

    // Release
    void Release(NNHandle handle);

    // Stats
    size_t GetCount() const;

private:
    NNObjectHandleRegistry m_ShaderAssets;
    NNObjectHandleRegistry m_Materials;
    NNObjectHandleRegistry m_MaterialInstances;
};
```

### Task 8: NNShaderSource — Shader Source Management
Create `NNRuntimeRenderAssets/Shader/NNShaderSource.h`

```cpp
class NNShaderSource : public INNObject {
public:
    // Load all shader files from directory
    static NNRef<NNShaderSource> LoadFromDirectory(const char* dir);

    // Get shader source by name
    const char* GetSource(const char* name) const;

    // Reload (for hot-reload)
    bool Reload();

private:
    struct ShaderEntry { std::string Name; std::string Source; };
    std::vector<ShaderEntry> m_Shaders;
    std::string m_Directory;
};
```

### Task 9: CMakeLists.txt
Create `NNRuntimeRenderAssets/CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 3.20)
project(NNRuntimeRenderAssets LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(NNRuntimeRenderAssets STATIC
    Source/Material/NNMaterial.cpp
    Source/Material/NNMaterialInstance.cpp
    Source/Shader/NNShaderAsset.cpp
    Source/Shader/NNShaderVariant.cpp
    Source/Shader/NNShaderSource.cpp
    Source/Cache/NNPipelineCache.cpp
    Source/Cache/NNAssetRegistry.cpp
)

target_include_directories(NNRuntimeRenderAssets PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(NNRuntimeRenderAssets PUBLIC
    NNRuntimeRender
    NNRuntimeCore
)
```

### Task 10: Test — TestMaterial.cpp
Create `NNRuntimeRenderAssets/Tests/TestMaterial.cpp`

Test cases:
1. Create material, set parameters
2. Create material instance, override parameters
3. Load shader asset from memory
4. Pipeline cache - verify PSO reuse
5. Asset registry - register/get/release handles

### Task 11: Integration
Update `Experiments/CMakeLists.txt`:
- Add `add_subdirectory(NNRuntimeRenderAssets)`
- Add Phase 6 test executable
- Link dependencies

## Verification
1. Build: `cmake --build . --target NNPhase6Test`
2. Run: `NNPhase6Test.exe`
3. Expected: All tests pass, PSO cache works correctly

## Next Steps (Phase 7)
- SRP (Scriptable Render Pipeline) interface
- C# material API via NNNativeEngineAPI
- Hot-reload for shaders
