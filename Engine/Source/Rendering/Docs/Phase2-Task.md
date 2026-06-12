# Phase 2 Task: Resource Layer Implementation

## Objective
Implement the Diligent Engine resource layer: Buffer, Texture, Sampler, RenderTarget. Fill in the NNDiligentDevice::Create* methods. Integrate HandleRegistry.

## Working Directory
`E:\Neverness\Engine\Source\Experiments`

## Key Constraint
- All Diligent API types must use fully qualified `::Diligent::` prefix
- NO `using namespace Diligent;` in ANY header files
- Use `using namespace NN::Runtime::Core;` and `using namespace NN::Runtime::Render;` ONLY inside .cpp files or class implementations where needed
- Match the existing code style in NNDiligentDevice.h/.cpp

## Existing Interface Definitions (already implemented, DO NOT modify)

- `NNRuntimeRender/Resources/INNBuffer.h` — INNBuffer interface with NNBufferDesc
- `NNRuntimeRender/Resources/INNTexture.h` — INNTexture interface with NNTextureDesc
- `NNRuntimeRender/Resources/INNSampler.h` — INNSampler interface with NNSamplerDesc
- `NNRuntimeRender/RenderTarget/INNRenderTarget.h` — INNRenderTarget interface with NNRenderTargetDesc
- `NNRuntimeRender/Device/INNRenderDevice.h` — INNRenderDevice with Create* virtual methods
- `NNRuntimeRender/NNRenderCore.h` — includes NNRef.h

## Existing Implementation (MODIFY these)

- `NNRuntimeDiligent/Device/NNDiligentDevice.h` — already has GetDiligentDevice/Context/SwapChain
- `NNRuntimeDiligent/Source/Device/NNDiligentDevice.cpp` — Create* methods currently return empty `{}`
- `NNRuntimeDiligent/CMakeLists.txt` — add new source files

## Files to CREATE

### 1. NNDiligentBuffer
- `NNRuntimeDiligent/Resources/NNDiligentBuffer.h`
- `NNRuntimeDiligent/Source/Resources/NNDiligentBuffer.cpp`

Implements INNBuffer. Wraps `::Diligent::IBuffer*`.
- Constructor takes `::Diligent::IBuffer*` and `NNBufferDesc`
- `UpdateData()` uses `::Diligent::IDeviceContext::UpdateBuffer()` or Map/UnMap
- For Constant buffers, use `::Diligent::USAGE::USAGE_DYNAMIC` + `MAP_WRITE`
- For Vertex/Index static, use `::Diligent::USAGE::USAGE_IMMUTABLE`
- `GetDesc()` returns stored desc
- `GetSize()` returns stored size
- AddRef/Release/GetRefCount via std::atomic

Diligent buffer creation mapping:
```
NNBufferType::Vertex   → Diligent::BIND_VERTEX_BUFFER
NNBufferType::Index    → Diligent::BIND_INDEX_BUFFER
NNBufferType::Constant → Diligent::BIND_UNIFORM_BUFFER
NNBufferType::Storage  → Diligent::BIND_SHADER_RESOURCE (for structured buffers)

NNBufferUsage::Static  → Diligent::USAGE_IMMUTABLE
NNBufferUsage::Dynamic → Diligent::USAGE_DYNAMIC
NNBufferUsage::Staging → Diligent::USAGE_STAGING
```

### 2. NNDiligentTexture
- `NNRuntimeDiligent/Resources/NNDiligentTexture.h`
- `NNRuntimeDiligent/Source/Resources/NNDiligentTexture.cpp`

Implements INNTexture. Wraps `::Diligent::ITexture*`.
- Constructor takes `::Diligent::ITexture*` and `NNTextureDesc`
- GetDesc/GetWidth/GetHeight return stored values
- `GetDiligentTexture()` accessor for internal use
- Handle RefCount

Diligent texture mapping:
```
NNTextureDimension::Tex2D   → Diligent::RESOURCE_DIM_TEX_2D
NNTextureDimension::Tex3D   → Diligent::RESOURCE_DIM_TEX_3D
NNTextureDimension::TexCube → Diligent::RESOURCE_DIM_TEX_CUBE
NNTextureDimension::Tex1D   → Diligent::RESOURCE_DIM_TEX_1D

NNTextureUsage::Default     → Diligent::USAGE_DEFAULT
NNTextureUsage::RenderTarget → Diligent::USAGE_DEFAULT + BIND_RENDER_TARGET
NNTextureUsage::DepthStencil → Diligent::USAGE_DEFAULT + BIND_DEPTH_STENCIL
NNTextureUsage::Staging     → Diligent::USAGE_STAGING

NNPixelFormat → Diligent::TEXTURE_FORMAT mapping:
RGBA8_UNORM    → TEX_FORMAT_RGBA8_UNORM
RGBA8_SRGB     → TEX_FORMAT_RGBA8_UNORM_SRGB
BGRA8_UNORM    → TEX_FORMAT_BGRA8_UNORM
R32_FLOAT      → TEX_FORMAT_R32_FLOAT
RG32_FLOAT     → TEX_FORMAT_RG32_FLOAT
RGBA32_FLOAT   → TEX_FORMAT_RGBA32_FLOAT
D32_FLOAT      → TEX_FORMAT_D32_FLOAT
D24_UNORM_S8_UINT → TEX_FORMAT_D24_UNORM_S8_UINT
BC1_UNORM      → TEX_FORMAT_BC1_UNORM
BC3_UNORM      → TEX_FORMAT_BC3_UNORM
BC5_UNORM      → TEX_FORMAT_BC5_UNORM
BC7_UNORM      → TEX_FORMAT_BC7_UNORM
```

### 3. NNDiligentSampler
- `NNRuntimeDiligent/Resources/NNDiligentSampler.h`
- `NNRuntimeDiligent/Source/Resources/NNDiligentSampler.cpp`

Implements INNSampler. Wraps `::Diligent::ISampler*`.
- Constructor takes `::Diligent::ISampler*` and `NNSamplerDesc`

Diligent sampler mapping:
```
NNFilterMode::Point       → Diligent::FILTER_TYPE_POINT
NNFilterMode::Linear      → Diligent::FILTER_TYPE_LINEAR
NNFilterMode::Anisotropic → Diligent::FILTER_TYPE_ANISOTROPIC

NNAddressMode::Wrap   → Diligent::TEXTURE_ADDRESS_WRAP
NNAddressMode::Clamp  → Diligent::TEXTURE_ADDRESS_CLAMP
NNAddressMode::Mirror → Diligent::TEXTURE_ADDRESS_MIRROR
NNAddressMode::Border → Diligent::TEXTURE_ADDRESS_BORDER
```

### 4. NNDiligentRenderTarget
- `NNRuntimeDiligent/Resources/NNDiligentRenderTarget.h`
- `NNRuntimeDiligent/Source/Resources/NNDiligentRenderTarget.cpp`

Implements INNRenderTarget. Manages color + depth textures.
- Stores array of `::Diligent::ITextureView*` for color attachments
- Stores `::Diligent::ITextureView*` for depth attachment
- Create internally: color texture + RTV, depth texture + DSV
- `GetDesc()`, `GetWidth()`, `GetHeight()` return stored values
- Accessors: `GetColorView(uint32_t index)`, `GetDepthView()`

### 5. Modify NNDiligentDevice
In `NNDiligentDevice.cpp`, fill in the Create* methods:

```cpp
NNRef<INNBuffer> NNDiligentDevice::CreateBuffer(const NNBufferDesc& desc, const void* initialData) {
    // Map NNBufferDesc → Diligent::BufferDesc
    // Call m_Device->CreateBuffer(bufDesc, initData, &diligentBuffer)
    // Wrap in NNDiligentBuffer
    // Return NNRef<INNBuffer>
}
```

Similarly for CreateTexture, CreateSampler, CreateRenderTarget.

For CreateShader and CreatePipelineState — leave as empty return `{}` for now (Phase 3).

### 6. Update CMakeLists.txt
Add new source files to NNRuntimeDiligent/CMakeLists.txt:
```cmake
add_library(NNRuntimeDiligent STATIC
    Source/Device/NNDiligentDevice.cpp
    Source/Command/NNDiligentCommandList.cpp
    Source/Resources/NNDiligentBuffer.cpp
    Source/Resources/NNDiligentTexture.cpp
    Source/Resources/NNDiligentSampler.cpp
    Source/Resources/NNDiligentRenderTarget.cpp
)
```

### 7. HandleRegistry Integration
The NNObjectHandleRegistry is in NNRuntimeCore. It's already implemented.
Add to NNNativeEngineAPI (create new file):
- `NNRuntimeDiligent/API/NNNativeEngineAPI.h`
- `NNRuntimeDiligent/Source/API/NNNativeEngineAPI.cpp`

This is a thin C API that:
- Holds a global NNObjectHandleRegistry
- NNE_CreateBuffer: calls device->CreateBuffer(), registers in registry, returns handle
- NNE_CreateTexture: same pattern
- NNE_ReleaseHandle: releases from registry

### 8. Test
- `NNRuntimeDiligent/Tests/TestResourceCreation.cpp`

Test that:
1. Device creates buffer (VB with triangle data) → returns valid NNRef
2. Device creates texture (2D 1024x1024 RGBA8) → returns valid NNRef
3. Device creates sampler → returns valid NNRef
4. Device creates render target → returns valid NNRef
5. HandleRegistry: register resource → get handle → get back resource → release
6. Run with NNRuntimeRenderBootstrap::CreateDevice (Vulkan or D3D12)

Update top-level CMakeLists.txt to add TestResourceCreation test executable.

## Build Command
```powershell
cd E:\Neverness\build
cmake --build . --target NNRuntimeDiligent --config Debug
cmake --build . --target NNPhase2Test --config Debug
```

## Important Notes
- The existing NNDiligentDevice.cpp uses `::Diligent::` fully qualified names everywhere. Follow this pattern.
- SDL3 is already linked for window creation.
- The NNRenderConfig.h and NNDiligentConfig.h handle platform detection and include common Diligent headers.
- Phase 1 test (NNPhase1Test) already works. Don't break it.
- Leave CreateShader and CreatePipelineState as stubs (return {}) — Phase 3 handles those.
