# Phase 3 Task: Shader + Pipeline + CommandList Implementation

## Objective
Implement shader compilation, PSO creation, fill in command list methods, and create a render queue. Verify with an integration test that renders a colored triangle through the full pipeline.

## Working Directory
`E:\Neverness\Engine\Source\Experiments`

## Architecture Reminder
```
NNRuntimeRender (interfaces only, no .cpp except RenderGraph)
    ↓
NNRuntimeDiligent (implements interfaces using Diligent Engine)
    ↓
NNRuntimeRenderBootstrap (factory)
    ↓
DiligentCore (third-party)
```

## Critical Constraints
- **NEVER** use `using namespace Diligent;` in any file
- **ALWAYS** use `::Diligent::` fully qualified prefix for Diligent types
- `using namespace NN::Runtime::Core;` and `using namespace NN::Runtime::Render;` are OK inside .cpp files and class bodies
- Follow existing code style in NNDiligentDevice.cpp and NNDiligentCommandList.cpp
- MSVC, C++20, Windows, UTF-8 with BOM or ASCII only (no Chinese characters in source)

## Existing Files (DO NOT modify interfaces)

### NNRuntimeRender (interfaces - read only)
- `Pipeline/INNShader.h` — INNShader with NNShaderDesc, NNShaderStage, NNShaderLanguage
- `Pipeline/INNPipelineState.h` — INNPipelineState with NNPipelineStateDesc (includes NNVertexLayout, NNRasterizerState, NNBlendState, NNDepthStencilState, NNPrimitiveTopology, PixelFormat)
- `Command/INNCommandList.h` — INNCommandList with Draw/DrawIndexed/Dispatch/SetPipelineState/SetVertexBuffer/SetIndexBuffer/SetViewports/SetScissorRects
- `Command/INNRenderQueue.h` — INNRenderQueue with Submit/Flush/GetPendingCount
- `Device/INNRenderDevice.h` — CreateShader/CreatePipelineState/CreateDeferredCommandList

### NNRuntimeDiligent (implementations - MODIFY these)
- `Device/NNDiligentDevice.h` — has GetDiligentDevice/GetDiligentContext/GetDiligentSwapChain
- `Source/Device/NNDiligentDevice.cpp` — CreateShader/CreatePipelineState are stubs returning {}
- `Command/NNDiligentCommandList.h` — already has ClearRenderTarget/Present
- `Source/Command/NNDiligentCommandList.cpp` — SetPipelineState/SetVertexBuffer/SetIndexBuffer/SetViewports/SetScissorRects are empty stubs; Draw/DrawIndexed/Dispatch/ClearRenderTarget/Present already implemented
- `CMakeLists.txt` — add new source files

### NNRuntimeRenderBootstrap
- `Include/NNRenderBootstrap.h` — NNRenderBootstrap::CreateDevice
- `Source/NNRenderBootstrap.cpp`

## Tasks

### Task 1: NNDiligentShader
Create:
- `NNRuntimeDiligent/Pipeline/NNDiligentShader.h`
- `NNRuntimeDiligent/Source/Pipeline/NNDiligentShader.cpp`

Implementation:
```cpp
class NNDiligentShader : public INNShader {
    ::Diligent::IShader* m_Shader = nullptr;
    NNShaderStage m_Stage;
    std::string m_DebugName;
    // ...
    ::Diligent::IShader* GetDiligentShader() const { return m_Shader; }
};
```

Constructor takes `::Diligent::IShader*` and stores it. The actual shader creation happens in NNDiligentDevice::CreateShader.

### Task 2: Implement NNDiligentDevice::CreateShader
In `Source/Device/NNDiligentDevice.cpp`, replace the stub:

```cpp
NNRef<INNShader> NNDiligentDevice::CreateShader(const NNShaderDesc& desc) {
    ::Diligent::ShaderCreateInfo ci;
    ci.SourceLanguage = ::Diligent::SHADER_SOURCE_LANGUAGE_HLSL;
    ci.Desc.UseCombinedTextureSamplers = true;
    
    // Map NNShaderStage to SHADER_TYPE
    switch (desc.Stage) {
        case NNShaderStage::Vertex:   ci.Desc.ShaderType = ::Diligent::SHADER_TYPE_VERTEX; break;
        case NNShaderStage::Pixel:    ci.Desc.ShaderType = ::Diligent::SHADER_TYPE_PIXEL; break;
        case NNShaderStage::Geometry: ci.Desc.ShaderType = ::Diligent::SHADER_TYPE_GEOMETRY; break;
        case NNShaderStage::Hull:     ci.Desc.ShaderType = ::Diligent::SHADER_TYPE_HULL; break;
        case NNShaderStage::Domain:   ci.Desc.ShaderType = ::Diligent::SHADER_TYPE_DOMAIN; break;
        case NNShaderStage::Compute:  ci.Desc.ShaderType = ::Diligent::SHADER_TYPE_COMPUTE; break;
    }
    
    ci.EntryPoint = desc.EntryPoint;
    ci.Desc.Name = desc.DebugName ? desc.DebugName : "NNShader";
    
    if (desc.SourceCode) {
        ci.Source = desc.SourceCode;
    } else if (desc.ByteCode) {
        ci.ByteCode = desc.ByteCode;
        ci.ByteCodeSize = desc.ByteCodeSize;
    }
    
    ::Diligent::IShader* shader = nullptr;
    m_Device->CreateShader(ci, nullptr, &shader);
    if (!shader) return {};
    
    auto* wrapper = new NNDiligentShader(shader, desc.Stage, desc.DebugName);
    wrapper->AddRef();
    shader->Release();
    return NNRef<INNShader>(wrapper);
}
```

NOTE: Diligent HLSL compilation requires shader compiler libraries (glslang/SPIRV). They should already be linked from Phase 1 since NNRuntimeDiligent links DiligentCore which includes the compiler.

**IMPORTANT**: For HLSL compilation on Vulkan, Diligent needs the HLSL source to use `#include` directives or have the `HLSL2GLSLConverterLib` linked. The simplest approach: use pre-compiled SPIR-V byte code or compile HLSL with `SHADER_SOURCE_LANGUAGE_HLSL` and set `ci.Desc.UseCombinedTextureSamplers = true`.

For the test, use simple HLSL vertex/pixel shaders (same ones from Phase 1 test).

### Task 3: Implement NNDiligentDevice::CreatePipelineState
Replace the stub:

```cpp
NNRef<INNPipelineState> NNDiligentDevice::CreatePipelineState(const NNPipelineStateDesc& desc) {
    // Get Diligent shader objects
    auto* vs = static_cast<NNDiligentShader*>(desc.VS);
    auto* ps = static_cast<NNDiligentShader*>(desc.PS);
    if (!vs || !ps) return {};
    
    ::Diligent::GraphicsPipelineStateCreateInfo psoCI;
    psoCI.PSODesc.Name = desc.DebugName ? desc.DebugName : "NNPSO";
    
    // Shaders
    psoCI.pVS = vs->GetDiligentShader();
    psoCI.pPS = ps->GetDiligentShader();
    
    // Input layout
    auto& layout = psoCI.GraphicsPipeline.InputLayout;
    std::vector<::Diligent::LayoutElement> elements;
    for (auto& attr : desc.VertexLayout.Attributes) {
        ::Diligent::LayoutElement elem;
        elem.HLSLSemantic = attr.SemanticName;
        elem.HLSLSemanticIndex = attr.SemanticIndex;
        elem.Location = attr.Location;
        elem.InputIndex = attr.Location; // same as Location
        elem.Offset = attr.Offset;
        elem.InputRate = (attr.InputRate == NNVertexInputRate::PerInstance) 
            ? ::Diligent::INPUT_RATE_INSTANCE : ::Diligent::INPUT_RATE_VERTEX;
        
        // Map format
        switch (attr.Format) {
            case NNVertexFormat::Float:  elem.ValueType = ::Diligent::VT_FLOAT32; elem.NumComponents = 1; break;
            case NNVertexFormat::Float2: elem.ValueType = ::Diligent::VT_FLOAT32; elem.NumComponents = 2; break;
            case NNVertexFormat::Float3: elem.ValueType = ::Diligent::VT_FLOAT32; elem.NumComponents = 3; break;
            case NNVertexFormat::Float4: elem.ValueType = ::Diligent::VT_FLOAT32; elem.NumComponents = 4; break;
            case NNVertexFormat::Int:    elem.ValueType = ::Diligent::VT_INT32;   elem.NumComponents = 1; break;
            case NNVertexFormat::Int2:   elem.ValueType = ::Diligent::VT_INT32;   elem.NumComponents = 2; break;
            case NNVertexFormat::Int3:   elem.ValueType = ::Diligent::VT_INT32;   elem.NumComponents = 3; break;
            case NNVertexFormat::Int4:   elem.ValueType = ::Diligent::VT_INT32;   elem.NumComponents = 4; break;
            case NNVertexFormat::UByte4_Norm: elem.ValueType = ::Diligent::VT_UINT8; elem.NumComponents = 4; elem.IsNormalized = true; break;
        }
        elements.push_back(elem);
    }
    layout.LayoutElements = elements.data();
    layout.NumElements = static_cast<Uint32>(elements.size());
    
    // Rasterizer
    auto& raster = psoCI.GraphicsPipeline.RasterizerDesc;
    raster.FillMode = (desc.RasterizerState.FillMode == NNFillMode::Wireframe) 
        ? ::Diligent::FILL_MODE_WIREFRAME : ::Diligent::FILL_MODE_SOLID;
    raster.CullMode = (desc.RasterizerState.CullMode == NNCullMode::None) 
        ? ::Diligent::CULL_MODE_NONE 
        : (desc.RasterizerState.CullMode == NNCullMode::Front) 
            ? ::Diligent::CULL_MODE_FRONT : ::Diligent::CULL_MODE_BACK;
    
    // Blend
    auto& blend = psoCI.GraphicsPipeline.BlendDesc;
    ::Diligent::RenderTargetBlendDesc rtBlend;
    rtBlend.BlendEnable = desc.BlendState.Enable;
    if (desc.BlendState.Enable) {
        auto MapBlendFactor = [](NNBlendFactor f) -> ::Diligent::BLEND_FACTOR {
            switch (f) {
                case NNBlendFactor::Zero:      return ::Diligent::BLEND_FACTOR_ZERO;
                case NNBlendFactor::One:       return ::Diligent::BLEND_FACTOR_ONE;
                case NNBlendFactor::SrcAlpha:  return ::Diligent::BLEND_FACTOR_SRC_ALPHA;
                case NNBlendFactor::InvSrcAlpha: return ::Diligent::BLEND_FACTOR_INV_SRC_ALPHA;
                default: return ::Diligent::BLEND_FACTOR_ONE;
            }
        };
        auto MapBlendOp = [](NNBlendOp op) -> ::Diligent::BLEND_OPERATION {
            switch (op) {
                case NNBlendOp::Add:      return ::Diligent::BLEND_OPERATION_ADD;
                case NNBlendOp::Subtract: return ::Diligent::BLEND_OPERATION_SUBTRACT;
                case NNBlendOp::Min:      return ::Diligent::BLEND_OPERATION_MIN;
                case NNBlendOp::Max:      return ::Diligent::BLEND_OPERATION_MAX;
                default: return ::Diligent::BLEND_OPERATION_ADD;
            }
        };
        rtBlend.SrcBlend = MapBlendFactor(desc.BlendState.SrcBlend);
        rtBlend.DestBlend = MapBlendFactor(desc.BlendState.DestBlend);
        rtBlend.BlendOp = MapBlendOp(desc.BlendState.BlendOp);
        rtBlend.SrcBlendAlpha = rtBlend.SrcBlend;
        rtBlend.DestBlendAlpha = rtBlend.DestBlend;
        rtBlend.BlendOpAlpha = rtBlend.BlendOp;
    }
    blend.RenderTargets[0] = rtBlend;
    
    // Depth stencil
    auto& ds = psoCI.GraphicsPipeline.DepthStencilDesc;
    ds.DepthEnable = desc.DepthStencilState.DepthEnable;
    ds.DepthWriteEnable = desc.DepthStencilState.DepthWriteEnable;
    auto MapCmpFunc = [](NNCompareFunc f) -> ::Diligent::COMPARISON_FUNCTION {
        switch (f) {
            case NNCompareFunc::Never:        return ::Diligent::COMPARISON_FUNC_NEVER;
            case NNCompareFunc::Less:         return ::Diligent::COMPARISON_FUNC_LESS;
            case NNCompareFunc::Equal:        return ::Diligent::COMPARISON_FUNC_EQUAL;
            case NNCompareFunc::LessEqual:    return ::Diligent::COMPARISON_FUNC_LESS_EQUAL;
            case NNCompareFunc::Greater:      return ::Diligent::COMPARISON_FUNC_GREATER;
            case NNCompareFunc::NotEqual:     return ::Diligent::COMPARISON_FUNC_NOT_EQUAL;
            case NNCompareFunc::GreaterEqual: return ::Diligent::COMPARISON_FUNC_GREATER_EQUAL;
            case NNCompareFunc::Always:       return ::Diligent::COMPARISON_FUNC_ALWAYS;
            default: return ::Diligent::COMPARISON_FUNC_LESS_EQUAL;
        }
    };
    ds.DepthFunc = MapCmpFunc(desc.DepthStencilState.DepthFunc);
    
    // Topology
    // NOTE: Diligent uses immediate topology on Draw calls, but PSO can specify primitive topology type
    // For simplicity, set PrimitiveType on PSO and use DrawAttribs topology at draw time
    
    // Render target formats
    psoCI.GraphicsPipeline.NumRenderTargets = 1;
    psoCI.GraphicsPipeline.RTVFormats[0] = ToDiligentFormat(desc.RTVFormat);  // reuse helper from device.cpp
    psoCI.GraphicsPipeline.DSVFormat = ToDiligentFormat(desc.DSVFormat);
    
    // Sample count
    psoCI.GraphicsPipeline.SmplDesc.Count = desc.SampleCount;
    
    // Subpass (use 0 for default)
    psoCI.GraphicsPipeline.SubpassIndex = 0;
    
    // For RenderPass-based rendering, need to provide pRenderPassNN
    // Since we're not using RenderGraph yet, create a default render pass
    // Actually, Diligent can create PSO without explicit RenderPass if using swapchain
    // We need to provide the render pass for proper PSO creation
    
    // Create a simple render pass for swap chain rendering
    ::Diligent::RenderPassAttachmentDesc passAttach;
    passAttach.Format = ToDiligentFormat(desc.RTVFormat);
    passAttach.SampleCount = desc.SampleCount;
    passAttach.LoadOp = ::Diligent::ATTACHMENT_LOAD_OP_CLEAR;
    passAttach.StoreOp = ::Diligent::ATTACHMENT_STORE_OP_STORE;
    passAttach.StencilLoadOp = ::Diligent::ATTACHMENT_LOAD_OP_DONT_CARE;
    passAttach.StencilStoreOp = ::Diligent::ATTACHMENT_STORE_OP_DONT_CARE;
    passAttach.InitialState = ::Diligent::RESOURCE_STATE_RENDER_TARGET;
    passAttach.FinalState = ::Diligent::RESOURCE_STATE_RENDER_TARGET;
    
    ::Diligent::RenderPassAttachmentDesc depthAttach;
    depthAttach.Format = ToDiligentFormat(desc.DSVFormat);
    depthAttach.SampleCount = desc.SampleCount;
    depthAttach.LoadOp = ::Diligent::ATTACHMENT_LOAD_OP_CLEAR;
    depthAttach.StoreOp = ::Diligent::ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttach.StencilLoadOp = ::Diligent::ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttach.StencilStoreOp = ::Diligent::ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttach.InitialState = ::Diligent::RESOURCE_STATE_DEPTH_WRITE;
    depthAttach.FinalState = ::Diligent::RESOURCE_STATE_DEPTH_WRITE;
    
    ::Diligent::SubpassDesc subpass;
    ::Diligent::AttachmentReference colorRef;
    colorRef.AttachmentIndex = 0;
    colorRef.State = ::Diligent::RESOURCE_STATE_RENDER_TARGET;
    subpass.RenderTargetAttachments = &colorRef;
    subpass.RenderTargetAttachmentCount = 1;
    ::Diligent::AttachmentReference depthRef;
    depthRef.AttachmentIndex = 1;
    depthRef.State = ::Diligent::RESOURCE_STATE_DEPTH_WRITE;
    subpass.DepthStencilAttachment = depthRef;
    
    ::Diligent::RenderPassDesc rpDesc;
    rpDesc.Name = "NNDefaultRenderPass";
    ::Diligent::RenderPassAttachmentDesc attachments[] = { passAttach, depthAttach };
    rpDesc.pAttachments = attachments;
    rpDesc.AttachmentCount = 2;
    rpDesc.pSubpasses = &subpass;
    rpDesc.SubpassCount = 1;
    
    ::Diligent::IRenderPass* renderPass = nullptr;
    m_Device->CreateRenderPass(rpDesc, &renderPass);
    if (renderPass) {
        psoCI.GraphicsPipeline.pRenderPassNN = renderPass;
        renderPass->Release(); // PSO holds a reference
    }
    
    ::Diligent::IPipelineState* pso = nullptr;
    m_Device->CreatePipelineState(psoCI, &pso);
    if (!pso) return {};
    
    auto* wrapper = new NNDiligentPipelineState(pso, desc);
    wrapper->AddRef();
    pso->Release();
    return NNRef<INNPipelineState>(wrapper);
}
```

**IMPORTANT NOTE about RenderPass**: Diligent Engine requires a RenderPass for PSO creation on Vulkan backend. The simplest approach for now:
1. Create a cached default RenderPass in NNDiligentDevice (for swap chain format)
2. Use it for all PSO creation until Phase 5 (RenderGraph)
3. Store it as `m_DefaultRenderPass` in NNDiligentDevice

Alternatively, check if the Phase 1 triangle test already creates a render pass — look at `TestTriangleDemo.cpp` for reference.

### Task 4: NNDiligentPipelineState
Create:
- `NNRuntimeDiligent/Pipeline/NNDiligentPipelineState.h`
- `NNRuntimeDiligent/Source/Pipeline/NNDiligentPipelineState.cpp`

Simple wrapper:
```cpp
class NNDiligentPipelineState : public INNPipelineState {
    ::Diligent::IPipelineState* m_PSO = nullptr;
    NNPipelineStateDesc m_Desc;
    // ...
    ::Diligent::IPipelineState* GetDiligentPSO() const { return m_PSO; }
};
```

### Task 5: Fill in NNDiligentCommandList Stubs
In `Source/Command/NNDiligentCommandList.cpp`:

```cpp
void NNDiligentCommandList::SetPipelineState(INNPipelineState* pso) {
    if (!pso) return;
    auto* diliPso = static_cast<NNDiligentPipelineState*>(pso);
    m_Device->GetDiligentContext()->SetPipelineState(diliPso->GetDiligentPSO());
}

void NNDiligentCommandList::SetVertexBuffer(INNBuffer* buffer, uint32_t slot) {
    if (!buffer) return;
    auto* diliBuf = static_cast<NNDiligentBuffer*>(buffer);
    ::Diligent::IBuffer* buf = diliBuf->GetDiligentBuffer();
    m_Device->GetDiligentContext()->SetVertexBuffers(slot, 1, &buf, nullptr, 
        ::Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, 
        ::Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
}

void NNDiligentCommandList::SetIndexBuffer(INNBuffer* buffer) {
    if (!buffer) return;
    auto* diliBuf = static_cast<NNDiligentBuffer*>(buffer);
    m_Device->GetDiligentContext()->SetIndexBuffer(diliBuf->GetDiligentBuffer(), 0, 
        ::Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
}

void NNDiligentCommandList::SetViewports(const NNViewport* viewports, uint32_t count) {
    if (!viewports || count == 0) return;
    std::vector<::Diligent::Viewport> vps(count);
    for (uint32_t i = 0; i < count; ++i) {
        vps[i].TopLeftX = viewports[i].TopLeftX;
        vps[i].TopLeftY = viewports[i].TopLeftY;
        vps[i].Width = viewports[i].Width;
        vps[i].Height = viewports[i].Height;
        vps[i].MinDepth = viewports[i].MinDepth;
        vps[i].MaxDepth = viewports[i].MaxDepth;
    }
    m_Device->GetDiligentContext()->SetViewports(vps.data(), count, 0, 0);
}

void NNDiligentCommandList::SetScissorRects(const NNRect* rects, uint32_t count) {
    if (!rects || count == 0) return;
    std::vector<::Diligent::Rect> r(count);
    for (uint32_t i = 0; i < count; ++i) {
        r[i].left = rects[i].Left;
        r[i].top = rects[i].Top;
        r[i].right = rects[i].Right;
        r[i].bottom = rects[i].Bottom;
    }
    m_Device->GetDiligentContext()->SetScissorRects(r.data(), count, 0, 0);
}
```

Also add `BeginRenderPass` and `EndRenderPass` to the command list for the integration test:
- `BeginRenderPass` should set up the swap chain RTV+DSV for rendering
- `EndRenderPass` is a no-op or transitions

Actually, looking at the current interface, there's no BeginRenderPass in INNCommandList. The Phase 1 test uses ClearRenderTarget + Draw + Present directly. Keep it simple — just fill in the existing stubs and add ClearRenderTarget/Present which are already there.

### Task 6: NNDiligentRenderQueue
Create:
- `NNRuntimeDiligent/Command/NNDiligentRenderQueue.h`
- `NNRuntimeDiligent/Source/Command/NNDiligentRenderQueue.cpp`

Simple implementation:
```cpp
class NNDiligentRenderQueue : public INNRenderQueue {
    NNDiligentDevice* m_Device;
    std::vector<INNCommandList*> m_PendingCmds;
    // Submit just calls Begin/End/Flush
    void Submit(INNCommandList* cmd) override {
        if (cmd) {
            cmd->AddRef();
            m_PendingCmds.push_back(cmd);
        }
    }
    void Flush() override {
        for (auto* cmd : m_PendingCmds) {
            // For immediate context, commands are already executed
            cmd->Release();
        }
        m_PendingCmds.clear();
    }
    uint32_t GetPendingCount() const override { return static_cast<uint32_t>(m_PendingCmds.size()); }
};
```

### Task 7: Update CMakeLists.txt
Add new source files to `NNRuntimeDiligent/CMakeLists.txt`:
```cmake
add_library(NNRuntimeDiligent STATIC
    Source/Device/NNDiligentDevice.cpp
    Source/Command/NNDiligentCommandList.cpp
    Source/Resources/NNDiligentBuffer.cpp
    Source/Resources/NNDiligentTexture.cpp
    Source/Resources/NNDiligentSampler.cpp
    Source/Resources/NNDiligentRenderTarget.cpp
    Source/Pipeline/NNDiligentShader.cpp
    Source/Pipeline/NNDiligentPipelineState.cpp
    Source/Command/NNDiligentRenderQueue.cpp
)
```

### Task 8: Integration Test — Phase3TriangleTest
Create `NNRuntimeDiligent/Tests/TestPhase3Triangle.cpp`

This test should:
1. Create device via NNRenderBootstrap
2. Create VS and PS shaders (HLSL source code)
3. Create vertex buffer with colored triangle data (position + color)
4. Create pipeline state with vertex layout
5. Run a render loop (100 frames):
   - ClearRenderTarget
   - SetPipelineState
   - SetVertexBuffer
   - SetViewports (full window)
   - Draw (3 vertices)
   - Present
6. Verify no crashes, output "PASS" at end

Use the same HLSL shaders from Phase 1 TestTriangleDemo.cpp:
```hlsl
// VS
struct PSInput { float4 Pos : SV_POSITION; float3 Color : COLOR; };
void main(in uint VertId : SV_VertexID, out PSInput PSIn) {
    float4 P[3]; P[0]=float4(-0.5,-0.5,0,1); P[1]=float4(0,0.5,0,1); P[2]=float4(0.5,-0.5,0,1);
    float3 C[3]; C[0]=float3(1,0,0); C[1]=float3(0,1,0); C[2]=float3(0,0,1);
    PSIn.Pos=P[VertId]; PSIn.Color=C[VertId];
}

// PS
struct PSInput { float4 Pos : SV_POSITION; float3 Color : COLOR; };
struct PSOutput { float4 Color : SV_TARGET; };
void main(in PSInput PSIn, out PSOutput PSOut) { PSOut.Color=float4(PSIn.Color.rgb,1); }
```

Add to top-level CMakeLists.txt:
```cmake
add_executable(NNPhase3Test NNRuntimeDiligent/Tests/TestPhase3Triangle.cpp)
target_include_directories(NNPhase3Test PRIVATE
    ${DILIGENT_ENGINE_DIR}/DiligentCore
    ${CMAKE_CURRENT_SOURCE_DIR}/NNRuntimeRenderBootstrap/Include
)
target_link_libraries(NNPhase3Test PRIVATE
    NNRuntimeDiligent NNRuntimeRenderBootstrap NNRuntimeRender NNRuntimeCore
    SDL3::SDL3 NevernessCore-Core
)
target_compile_definitions(NNPhase3Test PRIVATE PLATFORM_WIN32=1)
if(VULKAN_SUPPORTED)
    target_compile_definitions(NNPhase3Test PRIVATE VULKAN_SUPPORTED=1)
endif()
if(GL_SUPPORTED)
    target_compile_definitions(NNPhase3Test PRIVATE GL_SUPPORTED=1)
endif()
```

## Reference: Phase 1 TestTriangleDemo.cpp
Read `NNRuntimeDiligent/Tests/TestTriangleDemo.cpp` for reference on how the raw Diligent API was used in Phase 1. The Phase 3 test should achieve the same result but through our NNRuntimeRender interfaces.

## Reference: Diligent API Tips

### Shader Creation
```cpp
::Diligent::ShaderCreateInfo ci;
ci.SourceLanguage = ::Diligent::SHADER_SOURCE_LANGUAGE_HLSL;
ci.Desc.ShaderType = ::Diligent::SHADER_TYPE_VERTEX;
ci.EntryPoint = "main";
ci.Source = hlslSourceCode;
ci.Desc.Name = "MyVS";
ci.Desc.UseCombinedTextureSamplers = true;

::Diligent::IShader* shader = nullptr;
device->CreateShader(ci, nullptr, &shader);
```

### PSO Creation
```cpp
::Diligent::GraphicsPipelineStateCreateInfo psoCI;
psoCI.pVS = vsShader;
psoCI.pPS = psShader;
// ... fill in pipeline state ...
// Need RenderPass for Vulkan
device->CreatePipelineState(psoCI, &pso);
```

### RenderPass (for swap chain rendering)
```cpp
// Create a simple render pass matching swap chain format
::Diligent::RenderPassDesc rpDesc;
// ... 2 attachments: color + depth ...
device->CreateRenderPass(rpDesc, &renderPass);
psoCI.GraphicsPipeline.pRenderPassNN = renderPass;
```

### BeginRenderPass (for rendering to swap chain)
```cpp
::Diligent::BeginRenderPassAttribs beginRP;
beginRP.pRenderPass = renderPass;
// pRenderTargets, pDepthStencil, etc.
beginRP.ClearValueCount = 2;
beginRP.pClearValues = clearValues;
beginRP.StateTransitionMode = ::Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
ctx->BeginRenderPass(beginRP);
// ... draw ...
ctx->EndRenderPass();
```

## Build Command
```powershell
cd E:\Neverness\Build\Experiments-Test
cmake --build . --target NNRuntimeDiligent --config Debug
cmake --build . --target NNPhase3Test --config Debug
```

## Run Command
```powershell
& "E:\Neverness\Build\bin\Debug\NNPhase3Test.exe"
```

## Verification
- NNRuntimeDiligent library compiles without errors
- NNPhase3Test.exe links and runs
- Test creates device, shaders, PSO, VB, renders 100 frames, prints "PASS"
- No crashes, no validation errors
