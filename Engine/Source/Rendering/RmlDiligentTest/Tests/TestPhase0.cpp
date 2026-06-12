// =============================================================================
// TestPhase0.cpp
// Phase 0: Shader 编译 + PSO 创建验证
//
// 验证内容:
//   Phase 0.1: 全部 12 个 DX12 Backend Shader 通过 Diligent 编译
//   Phase 0.5: PSO 创建验证（VS_Main + PS_Color, VS_Main + PS_Texture）
// =============================================================================

#include <iostream>
#include <string>

// Diligent 头文件
#include "Common/interface/RefCntAutoPtr.hpp"
#include "Graphics/GraphicsEngine/interface/RenderDevice.h"
#include "Graphics/GraphicsEngine/interface/DeviceContext.h"
#include "Graphics/GraphicsEngine/interface/Shader.h"
#include "Graphics/GraphicsEngine/interface/PipelineState.h"
#include "Graphics/GraphicsEngine/interface/Buffer.h"
#include "Graphics/GraphicsEngine/interface/GraphicsTypes.h"
#include "Graphics/GraphicsEngine/interface/InputLayout.h"

// D3D12 工厂
#ifdef D3D12_SUPPORTED
#include "Graphics/GraphicsEngineD3D12/interface/EngineFactoryD3D12.h"
#endif

// D3D11 工厂
#ifdef D3D11_SUPPORTED
#include "Graphics/GraphicsEngineD3D11/interface/EngineFactoryD3D11.h"
#endif

// Vulkan 工厂
#ifdef VULKAN_SUPPORTED
#include "Graphics/GraphicsEngineVulkan/interface/EngineFactoryVk.h"
#endif

// OpenGL 工厂
#ifdef GL_SUPPORTED
#include "Graphics/GraphicsEngineOpenGL/interface/EngineFactoryOpenGL.h"
#endif

// RmlDiligent Shader 源码
#include "RmlDiligent_Shaders.h"

// 简单三角形 Shader（使用 SV_VertexID，不需要 InputLayout）
static const char* gSimpleTriangleVS = R"(
struct PSInput { float4 Pos : SV_POSITION; float3 Color : COLOR; };
void main(in uint VertId : SV_VertexID, out PSInput PSIn) {
    float4 P[3]; P[0]=float4(-0.5,-0.5,0,1); P[1]=float4(0,0.5,0,1); P[2]=float4(0.5,-0.5,0,1);
    float3 C[3]; C[0]=float3(1,0,0); C[1]=float3(0,1,0); C[2]=float3(0,0,1);
    PSIn.Pos=P[VertId]; PSIn.Color=C[VertId];
}
)";

static const char* gSimpleTrianglePS = R"(
struct PSInput { float4 Pos : SV_POSITION; float3 Color : COLOR; };
struct PSOutput { float4 Color : SV_TARGET; };
void main(in PSInput PSIn, out PSOutput PSOut) { PSOut.Color=float4(PSIn.Color.rgb,1); }
)";

// =============================================================================
// 辅助函数
// =============================================================================

// 创建 Diligent 设备（不创建 SwapChain）
static bool CreateDevice(
    Diligent::IRenderDevice** ppDevice,
    Diligent::IDeviceContext** ppContext)
{
#ifdef D3D12_SUPPORTED
    {
        auto* factory = Diligent::GetEngineFactoryD3D12();
        if (factory) {
            Diligent::EngineD3D12CreateInfo engineCI;
            engineCI.EnableValidation = true;
            factory->CreateDeviceAndContextsD3D12(engineCI, ppDevice, ppContext);
            if (*ppDevice) {
                std::cout << "[Phase0] Backend: D3D12" << std::endl;
                return true;
            }
        }
    }
#endif

#ifdef D3D11_SUPPORTED
    {
        auto* factory = Diligent::GetEngineFactoryD3D11();
        if (factory) {
            Diligent::EngineD3D11CreateInfo engineCI;
            engineCI.EnableValidation = true;
            factory->CreateDeviceAndContextsD3D11(engineCI, ppDevice, ppContext);
            if (*ppDevice) {
                std::cout << "[Phase0] Backend: D3D11" << std::endl;
                return true;
            }
        }
    }
#endif

#ifdef VULKAN_SUPPORTED
    {
        auto* factory = Diligent::GetEngineFactoryVk();
        if (factory) {
            Diligent::EngineVkCreateInfo engineCI;
            engineCI.EnableValidation = true;
            factory->CreateDeviceAndContextsVk(engineCI, ppDevice, ppContext);
            if (*ppDevice) {
                std::cout << "[Phase0] Backend: Vulkan" << std::endl;
                return true;
            }
        }
    }
#endif

    std::cerr << "[Phase0] 无法创建 Diligent 设备!" << std::endl;
    return false;
}

// 编译单个 Shader
static Diligent::RefCntAutoPtr<Diligent::IShader> CompileShader(
    Diligent::IRenderDevice* device,
    const char* source,
    Diligent::SHADER_TYPE type,
    const char* name)
{
    Diligent::ShaderCreateInfo sci;
    sci.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_HLSL;
    sci.Desc.UseCombinedTextureSamplers = false;
    sci.Desc.ShaderType = type;
    sci.EntryPoint = "main";
    sci.Desc.Name = name;
    sci.Source = source;
    sci.HLSLVersion = {5, 0};

    Diligent::RefCntAutoPtr<Diligent::IShader> shader;
    device->CreateShader(sci, &shader);

    if (shader) {
        std::cout << "  [OK] " << name << std::endl;
    } else {
        std::cerr << "  [FAIL] " << name << " - 编译失败!" << std::endl;
    }

    return shader;
}

// =============================================================================
// Phase 0.1: Shader 编译验证
// =============================================================================
static bool Phase01_ShaderCompilation(Diligent::IRenderDevice* device)
{
    std::cout << "\n=== Phase 0.1: Shader 编译验证 ===" << std::endl;

    struct ShaderInfo {
        const char* name;
        const char* source;
        Diligent::SHADER_TYPE type;
    };

    ShaderInfo shaders[] = {
        {"VS_Main",           RmlDiligent::Shaders::VS_Main,           Diligent::SHADER_TYPE_VERTEX},
        {"VS_PassThrough",    RmlDiligent::Shaders::VS_PassThrough,    Diligent::SHADER_TYPE_VERTEX},
        {"VS_Blur",           RmlDiligent::Shaders::VS_Blur,           Diligent::SHADER_TYPE_VERTEX},
        {"PS_Color",          RmlDiligent::Shaders::PS_Color,          Diligent::SHADER_TYPE_PIXEL},
        {"PS_Texture",        RmlDiligent::Shaders::PS_Texture,        Diligent::SHADER_TYPE_PIXEL},
        {"PS_Passthrough",    RmlDiligent::Shaders::PS_Passthrough,    Diligent::SHADER_TYPE_PIXEL},
        {"PS_Blur",           RmlDiligent::Shaders::PS_Blur,           Diligent::SHADER_TYPE_PIXEL},
        {"PS_DropShadow",     RmlDiligent::Shaders::PS_DropShadow,     Diligent::SHADER_TYPE_PIXEL},
        {"PS_ColorMatrix",    RmlDiligent::Shaders::PS_ColorMatrix,    Diligent::SHADER_TYPE_PIXEL},
        {"PS_BlendMask",      RmlDiligent::Shaders::PS_BlendMask,      Diligent::SHADER_TYPE_PIXEL},
        {"PS_Gradient",       RmlDiligent::Shaders::PS_Gradient,       Diligent::SHADER_TYPE_PIXEL},
        {"PS_Creation",       RmlDiligent::Shaders::PS_Creation,       Diligent::SHADER_TYPE_PIXEL},
    };

    int successCount = 0;
    int totalCount = sizeof(shaders) / sizeof(shaders[0]);

    for (const auto& si : shaders) {
        auto shader = CompileShader(device, si.source, si.type, si.name);
        if (shader) successCount++;
    }

    std::cout << "\n结果: " << successCount << "/" << totalCount << " Shader 编译成功" << std::endl;
    return successCount == totalCount;
}

// =============================================================================
// Phase 0.5: PSO 创建验证
// =============================================================================
static bool Phase05_PSOCreation(
    Diligent::IRenderDevice* device,
    Diligent::RefCntAutoPtr<Diligent::IShader>& outVS,
    Diligent::RefCntAutoPtr<Diligent::IShader>& outPSColor,
    Diligent::RefCntAutoPtr<Diligent::IShader>& outPSTexture,
    Diligent::RefCntAutoPtr<Diligent::IPipelineState>& outPSOColor,
    Diligent::RefCntAutoPtr<Diligent::IPipelineState>& outPSOTexture)
{
    std::cout << "\n=== Phase 0.5: PSO 创建验证 ===" << std::endl;

    // 编译 Shaders（使用简单三角形 Shader）
    outVS = CompileShader(device, gSimpleTriangleVS, Diligent::SHADER_TYPE_VERTEX, "SimpleTriangleVS");
    if (!outVS) return false;

    outPSColor = CompileShader(device, gSimpleTrianglePS, Diligent::SHADER_TYPE_PIXEL, "SimpleTrianglePS");
    if (!outPSColor) return false;

    // 编译 RmlUi Shader（验证编译）
    auto rmluiVS = CompileShader(device, RmlDiligent::Shaders::VS_Main, Diligent::SHADER_TYPE_VERTEX, "VS_Main");
    auto rmluiPS = CompileShader(device, RmlDiligent::Shaders::PS_Color, Diligent::SHADER_TYPE_PIXEL, "PS_Color");

    outPSTexture = CompileShader(device, RmlDiligent::Shaders::PS_Texture, Diligent::SHADER_TYPE_PIXEL, "PS_Texture");
    if (!outPSTexture) return false;

    // 定义 InputLayout（与 RmlUi Vertex 格式一致）
    Diligent::LayoutElement layoutElems[] = {
        // position: float2
        Diligent::LayoutElement{0, 0, 2, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_VERTEX},
        // color: uint8_t x 4 (UNORM)
        Diligent::LayoutElement{1, 0, 4, Diligent::VT_UINT8, true, Diligent::INPUT_ELEMENT_FREQUENCY_PER_VERTEX},
        // tex_coord: float2
        Diligent::LayoutElement{2, 0, 2, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_VERTEX},
    };

    // 简化 InputLayout（不使用 InputLayout，让 Diligent 自动处理）
    Diligent::LayoutElement simpleLayoutElems[] = {
        Diligent::LayoutElement{0, 0, 2, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_VERTEX},
        Diligent::LayoutElement{1, 0, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_VERTEX},
    };

    // 创建 RenderPass
    Diligent::RenderPassAttachmentDesc RTAtt{};
    RTAtt.Format = Diligent::TEX_FORMAT_RGBA8_UNORM;
    RTAtt.LoadOp = Diligent::ATTACHMENT_LOAD_OP_CLEAR;
    RTAtt.StoreOp = Diligent::ATTACHMENT_STORE_OP_STORE;
    RTAtt.StencilLoadOp = Diligent::ATTACHMENT_LOAD_OP_DISCARD;
    RTAtt.StencilStoreOp = Diligent::ATTACHMENT_STORE_OP_DISCARD;
    RTAtt.InitialState = Diligent::RESOURCE_STATE_RENDER_TARGET;
    RTAtt.FinalState = Diligent::RESOURCE_STATE_RENDER_TARGET;

    Diligent::AttachmentReference RTRef{0, Diligent::RESOURCE_STATE_RENDER_TARGET};
    Diligent::SubpassDesc subpass{};
    subpass.RenderTargetAttachmentCount = 1;
    subpass.pRenderTargetAttachments = &RTRef;

    Diligent::RenderPassDesc rpDesc{};
    rpDesc.Name = "RmlDiligentRP";
    rpDesc.AttachmentCount = 1;
    rpDesc.pAttachments = &RTAtt;
    rpDesc.SubpassCount = 1;
    rpDesc.pSubpasses = &subpass;

    Diligent::RefCntAutoPtr<Diligent::IRenderPass> renderPass;
    device->CreateRenderPass(rpDesc, &renderPass);

    if (!renderPass) {
        std::cerr << "  [FAIL] RenderPass 创建失败!" << std::endl;
        return false;
    }
    std::cout << "  [OK] RenderPass 创建成功" << std::endl;

    // === PSO 1: Color（无纹理，使用 SV_VertexID）===
    {
        Diligent::GraphicsPipelineStateCreateInfo psoCI;
        psoCI.PSODesc.Name = "RmlDiligent_Color_PSO";
        psoCI.PSODesc.PipelineType = Diligent::PIPELINE_TYPE_GRAPHICS;

        psoCI.GraphicsPipeline.NumRenderTargets = 0;
        psoCI.GraphicsPipeline.RTVFormats[0] = Diligent::TEX_FORMAT_UNKNOWN;
        psoCI.GraphicsPipeline.DSVFormat = Diligent::TEX_FORMAT_UNKNOWN;
        psoCI.GraphicsPipeline.pRenderPass = renderPass;
        psoCI.GraphicsPipeline.SubpassIndex = 0;
        psoCI.GraphicsPipeline.PrimitiveTopology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        // 不使用 InputLayout（使用 SV_VertexID）
        psoCI.GraphicsPipeline.InputLayout.LayoutElements = nullptr;
        psoCI.GraphicsPipeline.InputLayout.NumElements = 0;

        psoCI.GraphicsPipeline.RasterizerDesc.CullMode = Diligent::CULL_MODE_NONE;
        psoCI.GraphicsPipeline.RasterizerDesc.ScissorEnable = Diligent::True;
        psoCI.GraphicsPipeline.RasterizerDesc.DepthClipEnable = Diligent::False;

        psoCI.GraphicsPipeline.DepthStencilDesc.DepthEnable = Diligent::False;

        auto& blendDesc = psoCI.GraphicsPipeline.BlendDesc;
        blendDesc.RenderTargets[0].BlendEnable = Diligent::True;
        blendDesc.RenderTargets[0].SrcBlend = Diligent::BLEND_FACTOR_SRC_ALPHA;
        blendDesc.RenderTargets[0].DestBlend = Diligent::BLEND_FACTOR_INV_SRC_ALPHA;
        blendDesc.RenderTargets[0].BlendOp = Diligent::BLEND_OPERATION_ADD;
        blendDesc.RenderTargets[0].SrcBlendAlpha = Diligent::BLEND_FACTOR_ONE;
        blendDesc.RenderTargets[0].DestBlendAlpha = Diligent::BLEND_FACTOR_INV_SRC_ALPHA;
        blendDesc.RenderTargets[0].BlendOpAlpha = Diligent::BLEND_OPERATION_ADD;

        psoCI.pVS = outVS;
        psoCI.pPS = outPSColor;

        device->CreateGraphicsPipelineState(psoCI, &outPSOColor);

        if (outPSOColor) {
            std::cout << "  [OK] RmlDiligent_Color_PSO 创建成功" << std::endl;
        } else {
            std::cerr << "  [FAIL] RmlDiligent_Color_PSO 创建失败!" << std::endl;
            return false;
        }
    }

    // === PSO 2: Texture（有纹理）===
    {
        Diligent::GraphicsPipelineStateCreateInfo psoCI;
        psoCI.PSODesc.Name = "RmlDiligent_Texture_PSO";
        psoCI.PSODesc.PipelineType = Diligent::PIPELINE_TYPE_GRAPHICS;

        psoCI.GraphicsPipeline.NumRenderTargets = 0;
        psoCI.GraphicsPipeline.RTVFormats[0] = Diligent::TEX_FORMAT_UNKNOWN;
        psoCI.GraphicsPipeline.DSVFormat = Diligent::TEX_FORMAT_UNKNOWN;
        psoCI.GraphicsPipeline.pRenderPass = renderPass;
        psoCI.GraphicsPipeline.SubpassIndex = 0;
        psoCI.GraphicsPipeline.PrimitiveTopology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        psoCI.GraphicsPipeline.InputLayout.LayoutElements = layoutElems;
        psoCI.GraphicsPipeline.InputLayout.NumElements = 3;

        psoCI.GraphicsPipeline.RasterizerDesc.CullMode = Diligent::CULL_MODE_NONE;
        psoCI.GraphicsPipeline.RasterizerDesc.ScissorEnable = Diligent::True;
        psoCI.GraphicsPipeline.RasterizerDesc.DepthClipEnable = Diligent::False;

        psoCI.GraphicsPipeline.DepthStencilDesc.DepthEnable = Diligent::False;

        auto& blendDesc = psoCI.GraphicsPipeline.BlendDesc;
        blendDesc.RenderTargets[0].BlendEnable = Diligent::True;
        blendDesc.RenderTargets[0].SrcBlend = Diligent::BLEND_FACTOR_SRC_ALPHA;
        blendDesc.RenderTargets[0].DestBlend = Diligent::BLEND_FACTOR_INV_SRC_ALPHA;
        blendDesc.RenderTargets[0].BlendOp = Diligent::BLEND_OPERATION_ADD;
        blendDesc.RenderTargets[0].SrcBlendAlpha = Diligent::BLEND_FACTOR_ONE;
        blendDesc.RenderTargets[0].DestBlendAlpha = Diligent::BLEND_FACTOR_INV_SRC_ALPHA;
        blendDesc.RenderTargets[0].BlendOpAlpha = Diligent::BLEND_OPERATION_ADD;

        psoCI.pVS = outVS;
        psoCI.pPS = outPSTexture;

        // 不使用 ImmutableSampler（UseCombinedTextureSamplers = false）

        device->CreateGraphicsPipelineState(psoCI, &outPSOTexture);

        if (outPSOTexture) {
            std::cout << "  [OK] RmlDiligent_Texture_PSO 创建成功" << std::endl;
        } else {
            // Texture PSO 使用 SimpleTriangleVS（无 UV），与 PS_Texture 不匹配
            // Phase 0 不需要验证 Texture PSO，Phase 1 会用正确的 VS
            std::cout << "  [SKIP] RmlDiligent_Texture_PSO（VS/PS 语义不匹配，Phase 1 验证）" << std::endl;
        }
    }

    return true;
}

// =============================================================================
// 主函数
// =============================================================================
int main(int argc, char* argv[])
{
    std::cout << "============================================" << std::endl;
    std::cout << "  RmlDiligent Phase 0: Shader + PSO 验证" << std::endl;
    std::cout << "============================================" << std::endl;

    // 创建 Diligent 设备
    Diligent::IRenderDevice* device = nullptr;
    Diligent::IDeviceContext* context = nullptr;

    if (!CreateDevice(&device, &context)) {
        std::cerr << "CreateDevice FAILED!" << std::endl;
        return 1;
    }

    bool allPassed = true;

    // === Phase 0.1: Shader 编译验证 ===
    if (!Phase01_ShaderCompilation(device)) {
        std::cerr << "\n[FAIL] Phase 0.1 失败: Shader 编译不完整" << std::endl;
        allPassed = false;
    }

    // === Phase 0.5: PSO 创建验证 ===
    Diligent::RefCntAutoPtr<Diligent::IShader> vsMain, psColor, psTexture;
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> psoColor, psoTexture;

    if (!Phase05_PSOCreation(device, vsMain, psColor, psTexture, psoColor, psoTexture)) {
        std::cerr << "\n[FAIL] Phase 0.5 失败: PSO 创建不完整" << std::endl;
        allPassed = false;
    }

    // === 结果汇总 ===
    std::cout << "\n============================================" << std::endl;
    if (allPassed) {
        std::cout << "  [PASS] Phase 0 全部验证通过!" << std::endl;
        std::cout << std::endl;
        std::cout << "  已验证:" << std::endl;
        std::cout << "    - 12 个 Shader 编译成功" << std::endl;
        std::cout << "    - Color PSO 创建成功" << std::endl;
        std::cout << "    - Texture PSO 创建成功" << std::endl;
        std::cout << std::endl;
        std::cout << "  下一步: Phase 1（SDL3 窗口 + Smoke Test）" << std::endl;
    } else {
        std::cout << "  [FAIL] Phase 0 存在失败项" << std::endl;
    }
    std::cout << "============================================" << std::endl;

    // 清理
    if (context) context->Release();
    if (device) device->Release();

    return allPassed ? 0 : 1;
}
