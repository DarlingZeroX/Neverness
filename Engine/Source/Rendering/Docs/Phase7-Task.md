# Phase 7 Task: Scriptable Render Pipeline (SRP)

## Objective
Implement a C#-inheritable render pipeline system. C# layer can define custom rendering passes (shadow, forward, deferred, post-process) while the engine handles resource management and execution.

## Working Directory
`E:\Neverness\Engine\Source\Experiments`

## Critical Constraints
- NEVER use `using namespace Diligent;` - always `::Diligent::` fully qualified
- C++20, MSVC, Windows, ASCII only in source files
- SRP module has NO Diligent dependency - uses NNRuntimeRender interfaces
- Pipeline is data-driven: passes defined as structs, not hard-coded

## Architecture

```
NNRuntimeSRP (NEW - Scriptable Render Pipeline)
├── Pipeline/
│   ├── INNRenderPipeline.h       ← Abstract pipeline interface (C# inheritable)
│   ├── NNForwardPipeline.h       ← Default forward rendering
│   ├── NNDeferredPipeline.h      ← Default deferred rendering (stub)
│   └── NNPipelinePass.h          ← Single render pass definition
├── Context/
│   ├── NNRenderContext.h         ← Per-frame render context
│   ├── NNCameraData.h            ← Camera matrices/projection
│   └── NNSceneData.h             ← Lights, environment
├── Features/
│   ├── NNShadowPass.h            ← Shadow map generation
│   ├── NNForwardPass.h           ← Forward rendering pass
│   ├── NNPostProcessPass.h       ← Post-processing (stub)
│   └── NNImGuiPass.h             ← ImGui rendering
└── CSharp/
    └── NNCSRPBinding.h           ← C# SRP interop

Dependencies:
    NNRuntimeSRP → NNRuntimeRender (interfaces)
    NNRuntimeSRP → NNRuntimeRenderAssets (Material, ShaderAsset)
    NNRuntimeSRP → NNRuntimeCore (Handle, NNRef)
```

## Existing Modules (DO NOT modify)
- `NNRuntimeRender/Command/INNCommandList.h`
- `NNRuntimeRender/Device/INNRenderDevice.h`
- `NNRuntimeRender/RenderTarget/INNRenderTarget.h`
- `NNRuntimeRenderAssets/Material/NNMaterial.h`
- `NNRuntimeRenderAssets/Cache/NNPipelineCache.h`

## Tasks

### Task 1: NNPipelinePass — Pass Definition
```cpp
enum class NNPassType : uint8_t { Shadow, DepthPrePass, Forward, Deferred, PostProcess, Custom };

struct NNPipelinePassDesc {
    const char* Name;
    NNPassType Type;
    NNPixelFormat RTVFormat;
    NNPixelFormat DSVFormat;
    uint32_t Width, Height;
    bool ClearColor;
    bool ClearDepth;
    float ClearColorR, ClearColorG, ClearColorB, ClearColorA;
};
```

### Task 2: NNRenderContext — Per-frame Context
```cpp
struct NNCameraData {
    Matrix4x4 ViewMatrix;
    Matrix4x4 ProjMatrix;
    Matrix4x4 ViewProjMatrix;
    Vector3 Position;
    float NearPlane, FarPlane;
    float FOV;
};

struct NNLightData {
    Vector3 Direction;
    float Intensity;
    Vector3 Color;
    float ShadowBias;
    Matrix4x4 LightViewProj; // For shadow mapping
};

class NNRenderContext {
    NNCameraData Camera;
    std::vector<NNLightData> Lights;
    NNRenderDevice* Device;
    NNPipelineCache* PipelineCache;
    NNAssetRegistry* AssetRegistry;
    uint32_t FrameWidth, FrameHeight;
};
```

### Task 3: INNRenderPipeline — Pipeline Interface
```cpp
class INNRenderPipeline : public INNObject {
public:
    virtual bool Initialize(INNRenderDevice* device, uint32_t w, uint32_t h) = 0;
    virtual void Execute(INNCommandList* cmd, const NNRenderContext& ctx) = 0;
    virtual void Shutdown() = 0;
    virtual const char* GetName() const = 0;
    virtual uint32_t GetPassCount() const = 0;
};
```

### Task 4: NNForwardPipeline — Default Forward Rendering
```cpp
class NNForwardPipeline : public INNRenderPipeline {
    // Passes: Shadow → DepthPrePass → Forward → ImGui
    NNRef<INNRenderTarget> m_ShadowRT;
    NNRef<INNRenderTarget> m_ColorRT;
    NNRef<INNRenderTarget> m_DepthRT;
    std::vector<NNPipelinePassDesc> m_Passes;
};
```

### Task 5: NNShadowPass — Shadow Map Generation
```cpp
class NNShadowPass {
    void Setup(uint32_t shadowMapSize = 2048);
    void Execute(INNCommandList* cmd, const NNRenderContext& ctx,
                 const std::vector<NNMaterial*>& materials);
};
```

### Task 6: NNForwardPass — Forward Rendering
```cpp
class NNForwardPass {
    void Execute(INNCommandList* cmd, const NNRenderContext& ctx,
                 const std::vector<std::pair<NNMaterial*, INNBuffer*>>& drawCalls);
};
```

### Task 7: CMakeLists.txt + Test
- Create `NNRuntimeSRP/CMakeLists.txt`
- Create `NNRuntimeSRP/Tests/TestPipeline.cpp`
- Update `Experiments/CMakeLists.txt`

Test cases:
1. Pipeline pass description creation
2. Render context setup
3. Forward pipeline initialization
4. Pass execution order verification
5. Camera data consistency

## Verification
1. Build: `cmake --build . --target NNPhase7Test`
2. Run: `NNPhase7Test.exe`
3. Expected: All tests pass, pipeline structure validated

## Next Steps (Phase 8)
- C# interop via NNNativeEngineAPI
- Material property binding to constant buffers
- Scene graph integration
