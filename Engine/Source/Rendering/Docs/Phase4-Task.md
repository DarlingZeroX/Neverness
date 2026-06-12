# Phase 4 Task: Runtime Integration + NNNativeEngineAPI

## Objective
Create a C API layer (NNNativeEngineAPI) that exposes NNRuntimeRender interfaces as Handle-based functions for C# interop. Also create adapter stubs for VGFX and ImGui integration.

## Working Directory
`E:\Neverness\Engine\Source\Experiments`

## Critical Constraints
- NEVER use `using namespace Diligent;` - always `::Diligent::` fully qualified
- `using namespace NN::Runtime::Core;` and `using namespace NN::Runtime::Render;` OK in .cpp files
- C++20, MSVC, Windows, ASCII only in source files
- Follow existing code style in NNDiligentDevice.cpp

## Architecture Overview
```
C# / Managed Code
    ↓ (extern "C" Handle API)
NNNativeEngineAPI  ← NEW module
    ↓
NNRuntimeRender (interfaces)
    ↓
NNRuntimeDiligent (Diligent implementation)
    ↓
DiligentCore
```

## Existing Infrastructure
- `NNRuntimeCore/Handle/NNObjectHandleRegistry.h` — Register/Get/Release handles
- `NNRuntimeCore/Handle/NNHandleTypes.h` — NNRenderHandle, NNHandleType enum, MakeHandle/GetHandleType
- `NNRuntimeRender/Device/INNRenderDevice.h` — Create* methods
- `NNRuntimeRender/Resources/INNBuffer.h` — Buffer interface
- `NNRuntimeRender/Resources/INNTexture.h` — Texture interface
- `NNRuntimeRender/Resources/INNSampler.h` — Sampler interface
- `NNRuntimeRender/Pipeline/INNShader.h` — Shader interface
- `NNRuntimeRender/Pipeline/INNPipelineState.h` — PSO interface
- `NNRuntimeRender/Command/INNCommandList.h` — Command list interface
- `NNRuntimeRender/RenderTarget/INNRenderTarget.h` — Render target interface
- `NNRuntimeRenderBootstrap/Include/NNRenderBootstrap.h` — CreateDevice factory
- `NNRuntimeDiligent/Device/NNDiligentDevice.h` — GetDiligentDevice/Context/SwapChain
- `NNRuntimeDiligent/Command/NNDiligentCommandList.h` — ClearRenderTarget/Present

## Tasks

### Task 1: Create NNNativeEngineAPI Module Structure
Create directory: `NNRuntimeNativeEngineAPI/`

Files:
- `NNRuntimeNativeEngineAPI/CMakeLists.txt`
- `NNRuntimeNativeEngineAPI/API/NNEngineContext.h`
- `NNRuntimeNativeEngineAPI/API/NNRenderAPI.h`
- `NNRuntimeNativeEngineAPI/API/NNResourceAPI.h`
- `NNRuntimeNativeEngineAPI/Source/NNEngineContext.cpp`
- `NNRuntimeNativeEngineAPI/Source/NNRenderAPI.cpp`
- `NNRuntimeNativeEngineAPI/Source/NNResourceAPI.cpp`
- `NNRuntimeNativeEngineAPI/Tests/TestNativeAPI.cpp`

### Task 2: NNEngineContext — Global Engine State
`NNRuntimeNativeEngineAPI/API/NNEngineContext.h`:
```cpp
#pragma once

#include <NNRuntimeRender/Device/INNRenderDevice.h>
#include <NNRuntimeCore/Handle/NNObjectHandleRegistry.h>
#include <NNRuntimeCore/NNRef.h>

namespace NN::Runtime::NativeAPI
{
    using namespace NN::Runtime::Core;
    using namespace NN::Runtime::Render;

    // Singleton engine context that holds device + handle registry
    class NNEngineContext
    {
    public:
        static NNEngineContext& Get();

        bool Initialize(INNRenderDevice* device);
        void Shutdown();

        INNRenderDevice* GetDevice() { return m_Device.Get(); }
        NNObjectHandleRegistry& GetRegistry() { return m_Registry; }

        // Convenience: register a resource and return handle
        NNRenderHandle RegisterResource(NNHandleType type, INNObject* obj);
        INNObject* GetResource(NNRenderHandle handle);
        void ReleaseResource(NNRenderHandle handle);

    private:
        NNEngineContext() = default;
        NNRef<INNRenderDevice> m_Device;
        NNObjectHandleRegistry m_Registry;
    };
}
```

`Source/NNEngineContext.cpp`:
- Static singleton instance
- Initialize stores device ref
- RegisterResource wraps registry.Register with NNRef
- GetResource wraps registry.Get
- ReleaseResource wraps registry.Release

### Task 3: NNRenderAPI — C API for Rendering
`NNRuntimeNativeEngineAPI/API/NNRenderAPI.h`:
```cpp
#pragma once

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

// Handle type (same as NNRenderHandle but exposed to C)
typedef uint64_t NNE_Handle;
#define NNE_INVALID_HANDLE 0

// Forward declarations
typedef struct NNE_Device NNE_Device;
typedef struct NNE_Context NNE_Context;

// ===== Device =====
NNE_Device* NNE_GetDevice(void);
void NNE_SetDevice(NNE_Device* device);

// ===== Resource Creation (returns Handle) =====
NNE_Handle NNE_CreateBuffer(uint32_t type, uint32_t usage, uint32_t size, const void* initialData);
NNE_Handle NNE_CreateTexture(uint32_t width, uint32_t height, uint32_t format, uint32_t usage, const char* debugName);
NNE_Handle NNE_CreateSampler(uint32_t minFilter, uint32_t magFilter, uint32_t addressU, uint32_t addressV);
NNE_Handle NNE_CreateShader(uint32_t stage, const char* sourceCode, const char* entryPoint, const char* debugName);
NNE_Handle NNE_CreatePipelineState(NNE_Handle vs, NNE_Handle ps, const void* psoDesc);
NNE_Handle NNE_CreateRenderTarget(uint32_t width, uint32_t height, uint32_t colorFormat, uint32_t depthFormat);

// ===== Resource Destruction =====
void NNE_ReleaseHandle(NNE_Handle handle);

// ===== Query =====
uint32_t NNE_GetHandleType(NNE_Handle handle);
uint32_t NNE_IsHandleValid(NNE_Handle handle);

// ===== Render Commands =====
void NNE_ClearRenderTarget(float r, float g, float b, float a);
void NNE_SetPipelineState(NNE_Handle pso);
void NNE_SetVertexBuffer(NNE_Handle vb, uint32_t slot);
void NNE_SetIndexBuffer(NNE_Handle ib);
void NNE_SetViewport(float x, float y, float w, float h, float minD, float maxD);
void NNE_Draw(uint32_t vertexCount, uint32_t startVertex, uint32_t instanceCount);
void NNE_DrawIndexed(uint32_t indexCount, uint32_t startIndex, int32_t baseVertex, uint32_t instanceCount);
void NNE_Present(void);

#ifdef __cplusplus
}
#endif
```

### Task 4: NNResourceAPI — C API for Resource Queries
`NNRuntimeNativeEngineAPI/API/NNResourceAPI.h`:
```cpp
#pragma once

#include "NNRenderAPI.h"

#ifdef __cplusplus
extern "C" {
#endif

// ===== Buffer =====
uint32_t NNE_BufferGetSize(NNE_Handle buffer);
void NNE_BufferUpdateData(NNE_Handle buffer, const void* data, uint32_t size, uint32_t offset);

// ===== Texture =====
uint32_t NNE_TextureGetWidth(NNE_Handle texture);
uint32_t NNE_TextureGetHeight(NNE_Handle texture);

// ===== RenderTarget =====
uint32_t NNE_RenderTargetGetWidth(NNE_Handle rt);
uint32_t NNE_RenderTargetGetHeight(NNE_Handle rt);

#ifdef __cplusplus
}
#endif
```

### Task 5: Implement NNRenderAPI.cpp
Key implementation details:

```cpp
#include "../API/NNRenderAPI.h"
#include "../API/NNEngineContext.h"
#include <NNRuntimeRender/Resources/INNBuffer.h>
#include <NNRuntimeRender/Resources/INNTexture.h>
#include <NNRuntimeRender/Resources/INNSampler.h>
#include <NNRuntimeRender/Pipeline/INNShader.h>
#include <NNRuntimeRender/Pipeline/INNPipelineState.h>
#include <NNRuntimeRender/RenderTarget/INNRenderTarget.h>
#include <NNRuntimeDiligent/Device/NNDiligentDevice.h>
#include <NNRuntimeDiligent/Command/NNDiligentCommandList.h>

using namespace NN::Runtime::Core;
using namespace NN::Runtime::Render;
using namespace NN::Runtime::NativeAPI;

static NNE_Device* g_Device = nullptr;

extern "C" {

NNE_Device* NNE_GetDevice(void) { return g_Device; }
void NNE_SetDevice(NNE_Device* device) { g_Device = device; }

NNE_Handle NNE_CreateBuffer(uint32_t type, uint32_t usage, uint32_t size, const void* initialData)
{
    auto* ctx = NNEngineContext::Get().GetDevice();
    if (!ctx) return NNE_INVALID_HANDLE;

    NNBufferDesc desc{};
    desc.Type = static_cast<NNBufferType>(type);
    desc.Usage = static_cast<NNBufferUsage>(usage);
    desc.Size = size;

    auto buf = ctx->CreateBuffer(desc, initialData);
    if (!buf) return NNE_INVALID_HANDLE;

    return NNEngineContext::Get().RegisterResource(NNHandleType::Buffer, buf.Get());
}

NNE_Handle NNE_CreateTexture(uint32_t width, uint32_t height, uint32_t format, uint32_t usage, const char* debugName)
{
    auto* ctx = NNEngineContext::Get().GetDevice();
    if (!ctx) return NNE_INVALID_HANDLE;

    NNTextureDesc desc{};
    desc.Width = width;
    desc.Height = height;
    desc.Format = static_cast<NNPixelFormat>(format);
    desc.Usage = static_cast<NNTextureUsage>(usage);
    desc.DebugName = debugName;

    auto tex = ctx->CreateTexture(desc);
    if (!tex) return NNE_INVALID_HANDLE;

    return NNEngineContext::Get().RegisterResource(NNHandleType::Texture, tex.Get());
}

// ... similar for Sampler, Shader, PipelineState, RenderTarget ...

NNE_Handle NNE_CreateShader(uint32_t stage, const char* sourceCode, const char* entryPoint, const char* debugName)
{
    auto* ctx = NNEngineContext::Get().GetDevice();
    if (!ctx) return NNE_INVALID_HANDLE;

    NNShaderDesc desc{};
    desc.Stage = static_cast<NNShaderStage>(stage);
    desc.SourceCode = sourceCode;
    desc.EntryPoint = entryPoint;
    desc.DebugName = debugName;

    auto shader = ctx->CreateShader(desc);
    if (!shader) return NNE_INVALID_HANDLE;

    return NNEngineContext::Get().RegisterResource(NNHandleType::Shader, shader.Get());
}

NNE_Handle NNE_CreatePipelineState(NNE_Handle vs, NNE_Handle ps, const void* psoDescPtr)
{
    auto* device = NNEngineContext::Get().GetDevice();
    if (!device) return NNE_INVALID_HANDLE;

    auto* vsObj = dynamic_cast<INNShader*>(NNEngineContext::Get().GetResource(vs));
    auto* psObj = dynamic_cast<INNShader*>(NNEngineContext::Get().GetResource(ps));
    if (!vsObj || !psObj) return NNE_INVALID_HANDLE;

    // Use provided desc or default
    NNPipelineStateDesc psoDesc{};
    if (psoDescPtr) {
        psoDesc = *static_cast<const NNPipelineStateDesc*>(psoDescPtr);
    }
    psoDesc.VS = vsObj;
    psoDesc.PS = psObj;

    auto pso = device->CreatePipelineState(psoDesc);
    if (!pso) return NNE_INVALID_HANDLE;

    return NNEngineContext::Get().RegisterResource(NNHandleType::Pipeline, pso.Get());
}

void NNE_ReleaseHandle(NNE_Handle handle)
{
    NNEngineContext::Get().ReleaseResource(handle);
}

// ===== Render Commands =====
void NNE_ClearRenderTarget(float r, float g, float b, float a)
{
    auto* dev = static_cast<NNDiligent::NNDiligentDevice*>(NNEngineContext::Get().GetDevice());
    if (!dev) return;
    auto* cmd = static_cast<NNDiligent::NNDiligentCommandList*>(dev->GetImmediateCommandList());
    if (cmd) cmd->ClearRenderTarget(r, g, b, a);
}

void NNE_SetPipelineState(NNE_Handle pso)
{
    auto* dev = static_cast<NNDiligent::NNDiligentDevice*>(NNEngineContext::Get().GetDevice());
    if (!dev) return;
    auto* cmd = dev->GetImmediateCommandList();
    auto* psoObj = dynamic_cast<INNPipelineState*>(NNEngineContext::Get().GetResource(pso));
    if (cmd && psoObj) cmd->SetPipelineState(psoObj);
}

void NNE_Draw(uint32_t vertexCount, uint32_t startVertex, uint32_t instanceCount)
{
    auto* dev = static_cast<NNDiligent::NNDiligentDevice*>(NNEngineContext::Get().GetDevice());
    if (!dev) return;
    auto* cmd = dev->GetImmediateCommandList();
    if (!cmd) return;
    NNDrawAttribs da{};
    da.VertexCount = vertexCount;
    da.StartVertexLocation = startVertex;
    da.InstanceCount = instanceCount;
    cmd->Draw(da);
}

void NNE_Present(void)
{
    auto* dev = static_cast<NNDiligent::NNDiligentDevice*>(NNEngineContext::Get().GetDevice());
    if (!dev) return;
    auto* cmd = static_cast<NNDiligent::NNDiligentCommandList*>(dev->GetImmediateCommandList());
    if (cmd) cmd->Present();
}

} // extern "C"
```

### Task 6: CMakeLists.txt
```cmake
# NNRuntimeNativeEngineAPI

add_library(NNRuntimeNativeEngineAPI STATIC
    Source/NNEngineContext.cpp
    Source/NNRenderAPI.cpp
    Source/NNResourceAPI.cpp
)

target_include_directories(NNRuntimeNativeEngineAPI PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/API
)

target_link_libraries(NNRuntimeNativeEngineAPI PUBLIC
    NNRuntimeCore
    NNRuntimeRender
)

target_link_libraries(NNRuntimeNativeEngineAPI PRIVATE
    NNRuntimeDiligent
    NNRuntimeRenderBootstrap
)

target_compile_definitions(NNRuntimeNativeEngineAPI PRIVATE
    PLATFORM_WIN32=1
)
if(VULKAN_SUPPORTED)
    target_compile_definitions(NNRuntimeNativeEngineAPI PRIVATE VULKAN_SUPPORTED=1)
endif()
```

### Task 7: Test — TestNativeAPI.cpp
Create `NNRuntimeNativeEngineAPI/Tests/TestNativeAPI.cpp`:

Test that:
1. Create device via NNRenderBootstrap
2. Initialize NNEngineContext with device
3. NNE_CreateShader (VS + PS) → returns valid handles
4. NNE_CreateBuffer (VB with triangle data) → returns valid handle
5. NNE_CreatePipelineState(vs, ps) → returns valid handle
6. NNE_SetPipelineState, NNE_Draw, NNE_ClearRenderTarget, NNE_Present (5 frames)
7. NNE_ReleaseHandle for all resources
8. Verify Phase 2/3 tests still pass

Add to top-level CMakeLists.txt:
```cmake
add_executable(NNPhase4Test NNRuntimeNativeEngineAPI/Tests/TestNativeAPI.cpp)
target_include_directories(NNPhase4Test PRIVATE
    ${DILIGENT_ENGINE_DIR}/DiligentCore
    ${CMAKE_CURRENT_SOURCE_DIR}/NNRuntimeRenderBootstrap/Include
)
target_link_libraries(NNPhase4Test PRIVATE
    NNRuntimeNativeEngineAPI
    NNRuntimeDiligent
    NNRuntimeRenderBootstrap
    NNRuntimeRender
    NNRuntimeCore
    SDL3::SDL3 NevernessCore-Core
)
target_compile_definitions(NNPhase4Test PRIVATE PLATFORM_WIN32=1)
if(VULKAN_SUPPORTED)
    target_compile_definitions(NNPhase4Test PRIVATE VULKAN_SUPPORTED=1)
endif()
if(GL_SUPPORTED)
    target_compile_definitions(NNPhase4Test PRIVATE GL_SUPPORTED=1)
endif()
```

### Task 8: VGFX Adapter Stub
Create `NNRuntimeDiligent/Adapter/NNVGFXAdapter.h`:

A thin adapter that maps VGFX-style calls to NNRuntimeRender interface calls:
```cpp
#pragma once
// NNVGFXAdapter.h — Maps old VGFX rendering style to NNRuntimeRender interfaces
// This is a compatibility layer for migrating existing code

#include <NNRuntimeRender/Device/INNRenderDevice.h>
#include <NNRuntimeRender/Pipeline/INNPipelineState.h>
#include <NNRuntimeRender/Resources/INNBuffer.h>
#include <NNRuntimeRender/Resources/INNTexture.h>

namespace NNDiligent
{
    // High-level rendering helpers that use NNRuntimeRender interfaces
    // Similar to VGFX API style but backed by Diligent
    class NNVGFXAdapter
    {
    public:
        NNVGFXAdapter(NN::Runtime::Render::INNRenderDevice* device);
        ~NNVGFXAdapter();

        // Simple draw helpers (like VGFX)
        bool DrawTriangle(float x1, float y1, float x2, float y2, float x3, float y3,
                          float r, float g, float b);
        bool DrawQuad(float x, float y, float w, float h,
                      NN::Runtime::Core::NNRef<NN::Runtime::Render::INNTexture> texture);

    private:
        NN::Runtime::Render::INNRenderDevice* m_Device;
        // Cached PSOs for common operations
        NN::Runtime::Core::NNRef<NN::Runtime::Render::INNPipelineState> m_ColorPSO;
    };
}
```

## Build Command
```powershell
cd E:\Neverness\Build\Experiments-Test
cmake --build . --target NNRuntimeNativeEngineAPI --config Debug
cmake --build . --target NNPhase4Test --config Debug
```

## Run Command
```powershell
& "E:\Neverness\Build\bin\Debug\NNPhase4Test.exe"
```

## Verification
- NNRuntimeNativeEngineAPI compiles
- NNPhase4Test creates device, shaders, PSO, buffers all via C API handles
- Renders 5 frames via NNE_ClearRenderTarget + NNE_SetPipelineState + NNE_Draw + NNE_Present
- All handles properly released
- Phase 2/3 tests still pass
