// =============================================================================
// RmlDiligentRenderInterface.cpp
// RmlUi 的 Diligent Engine 渲染接口实现
//
// 关键设计决策（v4）:
// 1. BeginRenderPass + SwapChain Framebuffer
// 2. UseCombinedTextureSamplers = false（RmlUi HLSL 用独立 texture + sampler）
// 3. VS_Main + InputLayout 读取 RmlUi 顶点数据
// 4. 一 Geometry 一 Buffer（实验阶段）
// =============================================================================

#include "RmlDiligentRenderInterface.h"
#include "RmlDiligent_Shaders.h"
#include "RmlDiligentProgramId.h"
#include "RmlDiligentSrbCache.h"

#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/DecorationTypes.h>
#include <RmlUi/Core/FileInterface.h>
#include <RmlUi/Core/Log.h>
#include <RmlUi/Core/Matrix4.h>
#include <RmlUi/Core/SystemInterface.h>
#include <RmlUi/Core/Vertex.h>
#include <cstring>
#include <cstdio>
#include <cstdarg>
#include <iostream>
#include <memory>
#include <type_traits>

#ifdef RML_PERF_COUNTERS
#include <Windows.h>  // OutputDebugStringA
// 计时宏：记录 code 执行耗时，累加到 counterVar（微秒 tick）
#define RML_PERF_TIMED(counterVar, code) \
    { LARGE_INTEGER _pt0, _pt1; QueryPerformanceCounter(&_pt0); \
      code; QueryPerformanceCounter(&_pt1); \
      (counterVar) += _pt1.QuadPart - _pt0.QuadPart; }
#endif

// Diligent 头文件
#include "Graphics/GraphicsEngine/interface/DeviceContext.h"
#include "Graphics/GraphicsEngine/interface/Buffer.h"
#include "Graphics/GraphicsEngine/interface/Texture.h"
#include "Graphics/GraphicsEngine/interface/TextureView.h"
#include "Graphics/GraphicsEngine/interface/Sampler.h"
#include "Graphics/GraphicsTools/interface/MapHelper.hpp"

namespace RmlDiligent {

constexpr int kMaxNumStops = 16;

enum class ShaderGradientFunction {
    Linear = 0,
    Radial = 1,
    Conic = 2,
    RepeatingLinear = 3,
    RepeatingRadial = 4,
    RepeatingConic = 5,
};

enum class CompiledShaderType { Invalid = 0, Gradient, Creation };

struct CompiledShader {
    CompiledShaderType type = CompiledShaderType::Invalid;
    int gradientFunc = 0;
    Rml::Vector2f p;
    Rml::Vector2f v;
    Rml::Vector<float> stopPositions;
    Rml::Vector<Rml::Colourf> stopColors;
    Rml::Vector2f dimensions;
};

enum class FilterType { Invalid = 0, Passthrough, Blur, DropShadow, ColorMatrix, MaskImage };

struct CompiledFilter {
    FilterType type = FilterType::Invalid;
    float blend_factor = 1.0f;
    float sigma = 0.0f;
    Rml::Vector2f offset;
    Rml::ColourbPremultiplied color;
    Rml::Matrix4f color_matrix;
};

struct BlurCB {
    float transform[16];
    float translate[2];
    float texelOffset[2];
    float weights[4];
    float texCoordMin[2];
    float texCoordMax[2];
};
static_assert(sizeof(BlurCB) == 112, "BlurCB size must be 112 bytes");

struct DropShadowCBData {
    float texCoordMin[2];
    float texCoordMax[2];
    float color[4];
};
static_assert(sizeof(DropShadowCBData) == 32, "DropShadowCB size must be 32 bytes");

struct ColorMatrixCBData {
    float colorMatrix[16];
};
static_assert(sizeof(ColorMatrixCBData) == 64, "ColorMatrixCB size must be 64 bytes");

static void SigmaToParameters(float desired_sigma, int& out_pass_level, float& out_sigma)
{
    constexpr int max_num_passes = 10;
    constexpr float max_single_pass_sigma = 3.0f;
    out_pass_level = Rml::Math::Clamp(Rml::Math::Log2(static_cast<int>(desired_sigma * (2.f / max_single_pass_sigma))), 0, max_num_passes);
    out_sigma = Rml::Math::Clamp(desired_sigma / static_cast<float>(1 << out_pass_level), 0.0f, max_single_pass_sigma);
}

static void SetBlurWeights(float* weights, int num_weights, float sigma)
{
    float normalization = 0.0f;
    for (int i = 0; i < num_weights; ++i) {
        if (Rml::Math::Absolute(sigma) < 0.1f) {
            weights[i] = static_cast<float>(i == 0);
        } else {
            weights[i] = Rml::Math::Exp(-static_cast<float>(i * i) / (2.0f * sigma * sigma))
                / (Rml::Math::SquareRoot(2.f * Rml::Math::RMLUI_PI) * sigma);
        }
        normalization += (i == 0 ? 1.f : 2.0f) * weights[i];
    }
    for (int i = 0; i < num_weights; ++i) {
        weights[i] /= normalization;
    }
}

static void SetupOpacityBlendState(Diligent::GraphicsPipelineStateCreateInfo& psoCI)
{
    auto& blendDesc = psoCI.GraphicsPipeline.BlendDesc;
    auto& rt = blendDesc.RenderTargets[0];
    rt.RenderTargetWriteMask = Diligent::COLOR_MASK_ALL;
    rt.BlendEnable = Diligent::True;
    rt.SrcBlend = Diligent::BLEND_FACTOR_BLEND_FACTOR;
    rt.DestBlend = Diligent::BLEND_FACTOR_ZERO;
    rt.BlendOp = Diligent::BLEND_OPERATION_ADD;
    rt.SrcBlendAlpha = Diligent::BLEND_FACTOR_BLEND_FACTOR;
    rt.DestBlendAlpha = Diligent::BLEND_FACTOR_ZERO;
    rt.BlendOpAlpha = Diligent::BLEND_OPERATION_ADD;
}

static void SetupNoBlendState(Diligent::GraphicsPipelineStateCreateInfo& psoCI)
{
    auto& blendDesc = psoCI.GraphicsPipeline.BlendDesc;
    auto& rt = blendDesc.RenderTargets[0];
    rt.RenderTargetWriteMask = Diligent::COLOR_MASK_ALL;
    rt.BlendEnable = Diligent::False;
}

static Rml::Colourf ConvertToColorf(Rml::ColourbPremultiplied c0)
{
    Rml::Colourf result;
    for (int i = 0; i < 4; ++i) {
        result[i] = (1.f / 255.f) * static_cast<float>(c0[i]);
    }
    return result;
}

// RmlUi (top-left) ↔ window_flipped space used by RmlUi_Renderer_DX12 blur/blit.
static Rml::Rectanglei VerticallyFlipped(Rml::Rectanglei rect, int viewport_height)
{
    RMLUI_ASSERT(rect.Valid());
    Rml::Rectanglei flipped = rect;
    flipped.p0.y = viewport_height - rect.p1.y;
    flipped.p1.y = viewport_height - rect.p0.y;
    return flipped;
}

struct MainCB {
    float transform[16];
    float translate[2];
    float _padding[2];
};

struct GradientCB {
    float transform[16];
    float translate[2];
    int func;
    int numStops;
    float p[2];
    float v[2];
    float stopColors[16][4];
    float stopPositions[4][4];
};
static_assert(sizeof(GradientCB) == 416, "GradientCB size must be 416 bytes");

struct CreationCB {
    float transform[16];
    float translate[2];
    float dimensions[2];
    float value;
    float padding[3];
};
static_assert(sizeof(CreationCB) == 96, "CreationCB size must be 96 bytes");

static void UploadTransformToGradientCB(GradientCB* cb, const Rml::Matrix4f& transform, const Rml::Vector2f& translation)
{
    memcpy(cb->transform, transform.data(), sizeof(float) * 16);
    cb->translate[0] = translation.x;
    cb->translate[1] = translation.y;
}

static void UploadTransformToCreationCB(CreationCB* cb, const Rml::Matrix4f& transform, const Rml::Vector2f& translation)
{
    memcpy(cb->transform, transform.data(), sizeof(float) * 16);
    cb->translate[0] = translation.x;
    cb->translate[1] = translation.y;
}

#pragma pack(1)
struct TGAHeader {
    char idLength;
    char colourMapType;
    char dataType;
    char colourMapOrigin[2];
    char colourMapLength[2];
    char colourMapDepth;
    char xOrigin[2];
    char yOrigin[2];
    char width[2];
    char height[2];
    char bitsPerPixel;
    char imageDescriptor;
};
#pragma pack()

static void UploadTransformToCB(MainCB* cb, const Rml::Matrix4f& transform, const Rml::Vector2f& translation)
{
    memcpy(cb->transform, transform.data(), sizeof(float) * 16);
    cb->translate[0] = translation.x;
    cb->translate[1] = translation.y;
    cb->_padding[0] = 0.0f;
    cb->_padding[1] = 0.0f;
}

namespace {

// 诊断日志：默认禁用（文件 I/O 有 fopen/fprintf/fclose 开销）
// 取消注释 #define RML_DIAG_LOG 以启用
// #define RML_DIAG_LOG
void BoxShadowDiagLog(const char* fmt, ...)
{
#ifdef RML_DIAG_LOG
    FILE* file = std::fopen("boxshadow-diag.log", "a");
    if (!file) {
        return;
    }
    va_list args;
    va_start(args, fmt);
    std::vfprintf(file, fmt, args);
    va_end(args);
    std::fputc('\n', file);
    std::fclose(file);
#else
    (void)fmt;
#endif
}

} // namespace

// =============================================================================
// 构造/析构
// =============================================================================

RmlDiligentRenderInterface::RmlDiligentRenderInterface() = default;

RmlDiligentRenderInterface::~RmlDiligentRenderInterface() = default;

// =============================================================================
// 辅助函数：编译 Shader
// =============================================================================

static Diligent::RefCntAutoPtr<Diligent::IShader> CompileDiligentShader(
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

    if (!shader) {
        std::cerr << "[FAIL] Shader compile failed: " << name << std::endl;
    }

    return shader;
}

static void SetupBlendState(Diligent::GraphicsPipelineStateCreateInfo& psoCI, bool colorWriteEnabled = true)
{
    // RmlUi 输出预乘 alpha（与 RmlUi_Renderer_GL3 一致：glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA)）
    auto& blendDesc = psoCI.GraphicsPipeline.BlendDesc;
    auto& rt = blendDesc.RenderTargets[0];
    rt.RenderTargetWriteMask = colorWriteEnabled ? Diligent::COLOR_MASK_ALL : Diligent::COLOR_MASK_NONE;
    rt.BlendEnable = Diligent::True;
    rt.SrcBlend = Diligent::BLEND_FACTOR_ONE;
    rt.DestBlend = Diligent::BLEND_FACTOR_INV_SRC_ALPHA;
    rt.BlendOp = Diligent::BLEND_OPERATION_ADD;
    rt.SrcBlendAlpha = Diligent::BLEND_FACTOR_ONE;
    rt.DestBlendAlpha = Diligent::BLEND_FACTOR_INV_SRC_ALPHA;
    rt.BlendOpAlpha = Diligent::BLEND_OPERATION_ADD;
}

enum class StencilMode { Off, Equal, MaskReplace, MaskIncr };

static void ConfigureDepthStencil(Diligent::DepthStencilStateDesc& dss, StencilMode mode)
{
    dss.DepthEnable = Diligent::False;
    auto& ff = dss.FrontFace;
    ff.StencilDepthFailOp = Diligent::STENCIL_OP_KEEP;
    ff.StencilFailOp = Diligent::STENCIL_OP_KEEP;

    switch (mode) {
        case StencilMode::Off:
            dss.StencilEnable = Diligent::False;
            break;
        case StencilMode::Equal:
            dss.StencilEnable = Diligent::True;
            dss.StencilReadMask = 0xFF;
            dss.StencilWriteMask = 0xFF;
            ff.StencilFunc = Diligent::COMPARISON_FUNC_EQUAL;
            ff.StencilPassOp = Diligent::STENCIL_OP_KEEP;
            dss.BackFace = ff;
            break;
        case StencilMode::MaskReplace:
            dss.StencilEnable = Diligent::True;
            dss.StencilReadMask = 0xFF;
            dss.StencilWriteMask = 0xFF;
            ff.StencilFunc = Diligent::COMPARISON_FUNC_ALWAYS;
            ff.StencilPassOp = Diligent::STENCIL_OP_REPLACE;
            dss.BackFace = ff;
            break;
        case StencilMode::MaskIncr:
            dss.StencilEnable = Diligent::True;
            dss.StencilReadMask = 0xFF;
            dss.StencilWriteMask = 0xFF;
            ff.StencilFunc = Diligent::COMPARISON_FUNC_ALWAYS;
            ff.StencilPassOp = Diligent::STENCIL_OP_INCR_SAT;
            dss.BackFace = ff;
            break;
    }
}

// =============================================================================
// 初始化
// =============================================================================

bool RmlDiligentRenderInterface::Initialize(
    Diligent::IRenderDevice* device,
    Diligent::IDeviceContext* context,
    Diligent::ISwapChain* swapChain)
{
    m_Device = device;
    m_Context = context;
    m_SwapChain = swapChain;
    m_MsaaSamples = 2;

    CreateRenderPass(m_MsaaSamples);
    if (!m_RenderPass) {
        std::cerr << "[FAIL] RenderPass creation failed!" << std::endl;
        return false;
    }
    std::cout << "[OK] RenderPass created" << std::endl;

    m_RTPool.Initialize(device);
    m_LayerStack.Initialize(&m_RTPool);
    m_LayerStack.SetMsaaSamples(m_MsaaSamples);
    std::cout << "[OK] RenderTargetPool + LayerStack initialized" << std::endl;

    m_VS_Color = CompileDiligentShader(device, Shaders::VS_Main, Diligent::SHADER_TYPE_VERTEX, "VS_Main");
    m_VS_PassThrough = CompileDiligentShader(device, Shaders::VS_PassThrough, Diligent::SHADER_TYPE_VERTEX, "VS_PassThrough");
    m_PS_Color = CompileDiligentShader(device, Shaders::PS_Color, Diligent::SHADER_TYPE_PIXEL, "PS_Color");
    m_PS_Texture = CompileDiligentShader(device, Shaders::PS_Texture, Diligent::SHADER_TYPE_PIXEL, "PS_Texture");
    m_PS_Passthrough = CompileDiligentShader(device, Shaders::PS_Passthrough, Diligent::SHADER_TYPE_PIXEL, "PS_Passthrough");
    m_PS_Gradient = CompileDiligentShader(device, Shaders::PS_Gradient, Diligent::SHADER_TYPE_PIXEL, "PS_Gradient");
    m_PS_Creation = CompileDiligentShader(device, Shaders::PS_Creation, Diligent::SHADER_TYPE_PIXEL, "PS_Creation");
    m_VS_Blur = CompileDiligentShader(device, Shaders::VS_Blur, Diligent::SHADER_TYPE_VERTEX, "VS_Blur");
    m_PS_Blur = CompileDiligentShader(device, Shaders::PS_Blur, Diligent::SHADER_TYPE_PIXEL, "PS_Blur");
    m_PS_DropShadow = CompileDiligentShader(device, Shaders::PS_DropShadow, Diligent::SHADER_TYPE_PIXEL, "PS_DropShadow");
    m_PS_ColorMatrix = CompileDiligentShader(device, Shaders::PS_ColorMatrix, Diligent::SHADER_TYPE_PIXEL, "PS_ColorMatrix");
    m_PS_BlendMask = CompileDiligentShader(device, Shaders::PS_BlendMask, Diligent::SHADER_TYPE_PIXEL, "PS_BlendMask");
    m_VS_Texture = m_VS_Color;

    if (!m_VS_Color || !m_VS_PassThrough || !m_PS_Color || !m_PS_Texture || !m_PS_Passthrough || !m_PS_Gradient || !m_PS_Creation
        || !m_VS_Blur || !m_PS_Blur || !m_PS_DropShadow || !m_PS_ColorMatrix || !m_PS_BlendMask) {
        std::cerr << "[FAIL] Shader compile failed!" << std::endl;
        return false;
    }
    std::cout << "[OK] Shader compile success" << std::endl;

    {
        Diligent::SamplerDesc samplerDesc;
        samplerDesc.MinFilter = Diligent::FILTER_TYPE_LINEAR;
        samplerDesc.MagFilter = Diligent::FILTER_TYPE_LINEAR;
        samplerDesc.MipFilter = Diligent::FILTER_TYPE_LINEAR;
        samplerDesc.AddressU = Diligent::TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressV = Diligent::TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressW = Diligent::TEXTURE_ADDRESS_WRAP;
        m_Device->CreateSampler(samplerDesc, &m_Sampler);
        if (!m_Sampler) {
            std::cerr << "[FAIL] Sampler creation failed!" << std::endl;
            return false;
        }
    }

    CreatePSOs();

    if (!m_PSO_Color) {
        std::cerr << "[FAIL] Color PSO creation failed!" << std::endl;
        return false;
    }
    std::cout << "[OK] Color PSO created" << std::endl;

    if (!m_PSO_Texture) {
        std::cerr << "[FAIL] Texture PSO creation failed!" << std::endl;
        return false;
    }
    std::cout << "[OK] Texture PSO created" << std::endl;

    if (!m_PSO_Gradient) {
        std::cerr << "[FAIL] Gradient PSO creation failed!" << std::endl;
        return false;
    }
    std::cout << "[OK] Gradient PSO created" << std::endl;

    if (!m_PSO_Creation) {
        std::cerr << "[FAIL] Creation PSO creation failed!" << std::endl;
        return false;
    }
    std::cout << "[OK] Creation PSO created" << std::endl;

    if (!m_PSO_Color_StencilEqual || !m_PSO_Texture_StencilEqual || !m_PSO_Color_StencilSet || !m_PSO_Color_StencilIntersect) {
        std::cerr << "[FAIL] Stencil PSO creation failed!" << std::endl;
        return false;
    }
    std::cout << "[OK] Stencil PSOs created" << std::endl;

    if (!m_PSO_Passthrough || !m_PSO_Passthrough_StencilEqual || !m_PSO_PassthroughPresent || !m_PSO_PassthroughOpacity
        || !m_PSO_PassthroughReplace || !m_PSO_Blur || !m_PSO_DropShadow || !m_PSO_ColorMatrix || !m_PSO_BlendMask
        || !m_PSO_Composite || !m_PSO_CompositeReplace || !m_PSO_Composite_StencilEqual) {
        std::cerr << "[FAIL] Passthrough/Filter/Composite PSO creation failed!" << std::endl;
        return false;
    }
    std::cout << "[OK] Passthrough PSO created" << std::endl;
    std::cout << "[OK] Filter PSOs created" << std::endl;
    std::cout << "[OK] Composite PSOs created (MSAA " << m_MsaaSamples << "x)" << std::endl;

    CreateConstantBuffer();
    CreateShaderConstantBuffers();

    if (m_PSO_Color && m_ConstantBuffer) {
        m_PSO_Color->CreateShaderResourceBinding(&m_SRB_Color, true);
        if (m_SRB_Color) {
            if (auto* cbVar = m_SRB_Color->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, "ConstantBuffer")) {
                cbVar->Set(m_ConstantBuffer);
            }
            if (auto* projVar = m_SRB_Color->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, "ProjectionBuffer")) {
                projVar->Set(m_ProjectionCB);
            }
        }
    }
    // StencilEqual 变体需要独立 SRB（implicit signature 不同）
    if (m_PSO_Color_StencilEqual && m_ConstantBuffer) {
        m_PSO_Color_StencilEqual->CreateShaderResourceBinding(&m_SRB_Color_StencilEqual, true);
        if (m_SRB_Color_StencilEqual) {
            if (auto* cbVar = m_SRB_Color_StencilEqual->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, "ConstantBuffer")) {
                cbVar->Set(m_ConstantBuffer);
            }
            if (auto* projVar = m_SRB_Color_StencilEqual->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, "ProjectionBuffer")) {
                projVar->Set(m_ProjectionCB);
            }
        }
    }

    auto desc = swapChain->GetDesc();
    SetProjectionMatrix(desc.Width, desc.Height);

    std::remove("boxshadow-diag.log");
    BoxShadowDiagLog("=== box-shadow diagnostics (SaveLayer / CallbackDraw / bounds) ===");

    return true;
}

// =============================================================================
// 获取顶层 layer 纹理（离屏渲染用）
// =============================================================================

Diligent::ITexture* RmlDiligentRenderInterface::GetTopLayerTexture() const
{
    const auto& layer = m_LayerStack.GetTopLayer();
    return layer.texture.RawPtr();
}

// =============================================================================
// 离屏渲染：直接渲染到自定义 RT，绕过 LayerStack + swapchain
// =============================================================================

void RmlDiligentRenderInterface::BeginOffscreenFrame(
    Diligent::ITextureView* RTV, Diligent::ITextureView* DSV, int width, int height)
{
    // ResetPerfStats + 状态重置
    ResetPerfStats();
    m_SaveLayerAsTextureCount = 0;
    m_CallbackTextureDrawCount = 0;
    m_StateCache.Reset();
    m_SwapchainPassActive = false;
    m_ClipMaskEnabled = false;
    m_UseStencilEqual = false;
    m_StencilTestValue = 0;
    EnableClipMask(false);

    // 上传投影矩阵（m_Projection 已由 SetProjectionMatrix 设置）
    if (m_ProjectionCB && m_Context) {
        Diligent::MapHelper<float> cb(m_Context, m_ProjectionCB, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
        if (cb) {
            memcpy(cb, m_Projection.data(), sizeof(float) * 16);
        }
    }

    // 绑定自定义 RT（不使用 LayerStack）
    m_OffscreenRTV = RTV;
    m_OffscreenDSV = DSV;
    m_Context->SetRenderTargets(1, &RTV, DSV, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    // 清除
    const float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    m_Context->ClearRenderTarget(RTV, clearColor, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    if (DSV) {
        m_Context->ClearDepthStencil(DSV, Diligent::CLEAR_DEPTH_FLAG | Diligent::CLEAR_STENCIL_FLAG,
            1.0f, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    }

    // 视口
    Diligent::Viewport viewport;
    viewport.Width = static_cast<float>(width);
    viewport.Height = static_cast<float>(height);
    viewport.MaxDepth = 1.0f;
    m_Context->SetViewports(1, &viewport, 0, 0);

    m_ScissorEnabled = false;
    SetTransform(nullptr);
}

void RmlDiligentRenderInterface::EndOffscreenFrame()
{
    m_OffscreenRTV = nullptr;
    m_OffscreenDSV = nullptr;
    UnbindRenderTargets();
    m_StateCache.Reset();
}

// =============================================================================
// 在已有 RT 上叠加渲染 RmlUI（alpha 混合，不清除目标 RT）
// =============================================================================

void RmlDiligentRenderInterface::CompositeOnTop(
    Diligent::ITextureView* RTV, Diligent::ITextureView* DSV, int width, int height)
{
    // 状态重置（与 BeginOffscreenFrame 相同，但不清除 RT）
    ResetPerfStats();
    m_SaveLayerAsTextureCount = 0;
    m_CallbackTextureDrawCount = 0;
    m_StateCache.Reset();
    m_SwapchainPassActive = false;
    m_ClipMaskEnabled = false;
    m_UseStencilEqual = false;
    m_StencilTestValue = 0;
    EnableClipMask(false);

    // 上传投影矩阵（m_Projection 已由 SetProjectionMatrix 设置）
    if (m_ProjectionCB && m_Context) {
        Diligent::MapHelper<float> cb(m_Context, m_ProjectionCB, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
        if (cb) {
            memcpy(cb, m_Projection.data(), sizeof(float) * 16);
        }
    }

    // 绑定 Scene RT（不清除，保留场景内容）
    m_OffscreenRTV = RTV;
    m_OffscreenDSV = DSV;
    m_Context->SetRenderTargets(1, &RTV, DSV, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    // 视口
    Diligent::Viewport viewport;
    viewport.Width = static_cast<float>(width);
    viewport.Height = static_cast<float>(height);
    viewport.MaxDepth = 1.0f;
    m_Context->SetViewports(1, &viewport, 0, 0);

    m_ScissorEnabled = false;
    SetTransform(nullptr);

    // IB 状态转换（首次从 COPY_DEST → INDEX_BUFFER，后续自动检测当前状态）
    if (m_FullscreenIB) {
        Diligent::StateTransitionDesc transitions[1];
        transitions[0].pResource = m_FullscreenIB;
        transitions[0].OldState = Diligent::RESOURCE_STATE_UNKNOWN;
        transitions[0].NewState = Diligent::RESOURCE_STATE_INDEX_BUFFER;
        transitions[0].Flags = Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE;
        m_Context->TransitionResourceStates(1, transitions);
    }
}

// =============================================================================
// 创建 RenderPass
// =============================================================================

void RmlDiligentRenderInterface::CreateRenderPass(int msaa_samples)
{
    Diligent::RenderPassAttachmentDesc attachments[2]{};

    attachments[0].Format = m_SwapChain->GetDesc().ColorBufferFormat;
    attachments[0].SampleCount = static_cast<Diligent::Uint32>(msaa_samples);
    attachments[0].LoadOp = Diligent::ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].StoreOp = Diligent::ATTACHMENT_STORE_OP_STORE;
    attachments[0].InitialState = Diligent::RESOURCE_STATE_RENDER_TARGET;
    attachments[0].FinalState = Diligent::RESOURCE_STATE_RENDER_TARGET;

    attachments[1].Format = Diligent::TEX_FORMAT_D24_UNORM_S8_UINT;
    attachments[1].SampleCount = static_cast<Diligent::Uint32>(msaa_samples);
    attachments[1].LoadOp = Diligent::ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].StoreOp = Diligent::ATTACHMENT_STORE_OP_STORE;
    attachments[1].StencilLoadOp = Diligent::ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].StencilStoreOp = Diligent::ATTACHMENT_STORE_OP_STORE;
    attachments[1].InitialState = Diligent::RESOURCE_STATE_DEPTH_WRITE;
    attachments[1].FinalState = Diligent::RESOURCE_STATE_DEPTH_WRITE;

    Diligent::AttachmentReference RTRef{0, Diligent::RESOURCE_STATE_RENDER_TARGET};
    Diligent::AttachmentReference DSRef{1, Diligent::RESOURCE_STATE_DEPTH_WRITE};
    Diligent::SubpassDesc subpass{};
    subpass.RenderTargetAttachmentCount = 1;
    subpass.pRenderTargetAttachments = &RTRef;
    subpass.pDepthStencilAttachment = &DSRef;

    Diligent::RenderPassDesc rpDesc{};
    rpDesc.Name = "RmlDiligentRP";
    rpDesc.AttachmentCount = 2;
    rpDesc.pAttachments = attachments;
    rpDesc.SubpassCount = 1;
    rpDesc.pSubpasses = &subpass;

    m_Device->CreateRenderPass(rpDesc, &m_RenderPass);
}

// =============================================================================
// 创建 PSO
// =============================================================================

void RmlDiligentRenderInterface::CreatePSOs()
{
    // HLSLSemantic 必须与 VS_Main 中的语义名一致（默认 "ATTRIB" 会导致 D3D12 PSO 创建失败）
    Diligent::LayoutElement layoutElems[] = {
        Diligent::LayoutElement{"POSITION", 0, 0, 2, Diligent::VT_FLOAT32, Diligent::False,
                                Diligent::LAYOUT_ELEMENT_AUTO_OFFSET, Diligent::LAYOUT_ELEMENT_AUTO_STRIDE,
                                Diligent::INPUT_ELEMENT_FREQUENCY_PER_VERTEX},
        Diligent::LayoutElement{"COLOR", 0, 0, 4, Diligent::VT_UINT8, Diligent::True,
                                Diligent::LAYOUT_ELEMENT_AUTO_OFFSET, Diligent::LAYOUT_ELEMENT_AUTO_STRIDE,
                                Diligent::INPUT_ELEMENT_FREQUENCY_PER_VERTEX},
        Diligent::LayoutElement{"TEXCOORD", 0, 0, 2, Diligent::VT_FLOAT32, Diligent::False,
                                Diligent::LAYOUT_ELEMENT_AUTO_OFFSET, Diligent::LAYOUT_ELEMENT_AUTO_STRIDE,
                                Diligent::INPUT_ELEMENT_FREQUENCY_PER_VERTEX},
    };

    Diligent::ShaderResourceVariableDesc colorVars[] = {
        {Diligent::SHADER_TYPE_VERTEX, "ConstantBuffer", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        {Diligent::SHADER_TYPE_VERTEX, "ProjectionBuffer", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
    };

    Diligent::ShaderResourceVariableDesc textureVars[] = {
        {Diligent::SHADER_TYPE_VERTEX, "ConstantBuffer", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        {Diligent::SHADER_TYPE_VERTEX, "ProjectionBuffer", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        {Diligent::SHADER_TYPE_PIXEL, "g_InputTexture", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        {Diligent::SHADER_TYPE_PIXEL, "g_SamplerLinear", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
    };

    auto setupCommonPipeline = [&](Diligent::GraphicsPipelineStateCreateInfo& psoCI, StencilMode stencilMode,
                                   bool colorWriteEnabled = true, int samples = 1) {
        psoCI.PSODesc.PipelineType = Diligent::PIPELINE_TYPE_GRAPHICS;
        // 默认显式格式（兼容 offscreen RT 和 swapchain，消除 validation warning）
        psoCI.GraphicsPipeline.NumRenderTargets = 1;
        psoCI.GraphicsPipeline.RTVFormats[0] = Diligent::TEX_FORMAT_RGBA8_UNORM;
        psoCI.GraphicsPipeline.DSVFormat = Diligent::TEX_FORMAT_D24_UNORM_S8_UINT;
        psoCI.GraphicsPipeline.pRenderPass = nullptr;
        psoCI.GraphicsPipeline.SubpassIndex = 0;
        psoCI.GraphicsPipeline.PrimitiveTopology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        psoCI.GraphicsPipeline.InputLayout.LayoutElements = layoutElems;
        psoCI.GraphicsPipeline.InputLayout.NumElements = 3;
        psoCI.GraphicsPipeline.RasterizerDesc.CullMode = Diligent::CULL_MODE_NONE;
        psoCI.GraphicsPipeline.RasterizerDesc.ScissorEnable = Diligent::True;
        psoCI.GraphicsPipeline.RasterizerDesc.DepthClipEnable = Diligent::True;
        psoCI.GraphicsPipeline.SmplDesc.Count = static_cast<Diligent::Uint32>(samples);
        ConfigureDepthStencil(psoCI.GraphicsPipeline.DepthStencilDesc, stencilMode);
        SetupBlendState(psoCI, colorWriteEnabled);
    };

    // Layer draws bind RGBA8 color + D24S8 (shared layer depth/stencil). PSO formats must match — unlike
    // m_RenderPass-based defaults (NumRenderTargets=0 / DSVFormat=UNKNOWN) which break stencil writes on D3D12.
    auto applyLayerOutputFormats = [](Diligent::GraphicsPipelineStateCreateInfo& psoCI,
                                      Diligent::TEXTURE_FORMAT dsvFormat = Diligent::TEX_FORMAT_D24_UNORM_S8_UINT) {
        psoCI.GraphicsPipeline.pRenderPass = nullptr;
        psoCI.GraphicsPipeline.NumRenderTargets = 1;
        psoCI.GraphicsPipeline.RTVFormats[0] = Diligent::TEX_FORMAT_RGBA8_UNORM;
        psoCI.GraphicsPipeline.DSVFormat = dsvFormat;
    };

    {
        Diligent::GraphicsPipelineStateCreateInfo psoCI;
        psoCI.PSODesc.Name = "RmlDiligent_Color_PSO";
        psoCI.PSODesc.ResourceLayout.Variables = colorVars;
        psoCI.PSODesc.ResourceLayout.NumVariables = _countof(colorVars);
        setupCommonPipeline(psoCI, StencilMode::Off, true, m_MsaaSamples);
        applyLayerOutputFormats(psoCI);
        psoCI.pVS = m_VS_Color;
        psoCI.pPS = m_PS_Color;
        m_Device->CreateGraphicsPipelineState(psoCI, &m_PSO_Color);
    }

    {
        Diligent::GraphicsPipelineStateCreateInfo psoCI;
        psoCI.PSODesc.Name = "RmlDiligent_Texture_PSO";
        psoCI.PSODesc.ResourceLayout.Variables = textureVars;
        psoCI.PSODesc.ResourceLayout.NumVariables = _countof(textureVars);
        setupCommonPipeline(psoCI, StencilMode::Off, true, m_MsaaSamples);
        applyLayerOutputFormats(psoCI);
        psoCI.pVS = m_VS_Color;
        psoCI.pPS = m_PS_Texture;
        m_Device->CreateGraphicsPipelineState(psoCI, &m_PSO_Texture);
    }

    {
        Diligent::ShaderResourceVariableDesc shaderCBVars[] = {
            {Diligent::SHADER_TYPE_VERTEX, "ConstantBuffer", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {Diligent::SHADER_TYPE_VERTEX, "ProjectionBuffer", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {Diligent::SHADER_TYPE_PIXEL, "SharedConstantBuffer", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        };
        Diligent::GraphicsPipelineStateCreateInfo psoCI;
        psoCI.PSODesc.Name = "RmlDiligent_Gradient_PSO";
        psoCI.PSODesc.ResourceLayout.Variables = shaderCBVars;
        psoCI.PSODesc.ResourceLayout.NumVariables = _countof(shaderCBVars);
        setupCommonPipeline(psoCI, StencilMode::Off, true, m_MsaaSamples);
        psoCI.GraphicsPipeline.pRenderPass = nullptr;
        psoCI.GraphicsPipeline.NumRenderTargets = 1;
        psoCI.GraphicsPipeline.RTVFormats[0] = Diligent::TEX_FORMAT_RGBA8_UNORM;
        psoCI.GraphicsPipeline.DSVFormat = Diligent::TEX_FORMAT_D24_UNORM_S8_UINT;
        psoCI.pVS = m_VS_Color;
        psoCI.pPS = m_PS_Gradient;
        m_Device->CreateGraphicsPipelineState(psoCI, &m_PSO_Gradient);
    }

    {
        Diligent::ShaderResourceVariableDesc shaderCBVars[] = {
            {Diligent::SHADER_TYPE_VERTEX, "ConstantBuffer", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {Diligent::SHADER_TYPE_PIXEL, "SharedConstantBuffer", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        };
        Diligent::GraphicsPipelineStateCreateInfo psoCI;
        psoCI.PSODesc.Name = "RmlDiligent_Creation_PSO";
        psoCI.PSODesc.ResourceLayout.Variables = shaderCBVars;
        psoCI.PSODesc.ResourceLayout.NumVariables = _countof(shaderCBVars);
        setupCommonPipeline(psoCI, StencilMode::Off, true, m_MsaaSamples);
        psoCI.GraphicsPipeline.pRenderPass = nullptr;
        psoCI.GraphicsPipeline.NumRenderTargets = 1;
        psoCI.GraphicsPipeline.RTVFormats[0] = Diligent::TEX_FORMAT_RGBA8_UNORM;
        psoCI.GraphicsPipeline.DSVFormat = Diligent::TEX_FORMAT_D24_UNORM_S8_UINT;
        psoCI.pVS = m_VS_Color;
        psoCI.pPS = m_PS_Creation;
        m_Device->CreateGraphicsPipelineState(psoCI, &m_PSO_Creation);
    }

    {
        Diligent::GraphicsPipelineStateCreateInfo psoCI;
        psoCI.PSODesc.Name = "RmlDiligent_Color_StencilEqual_PSO";
        psoCI.PSODesc.ResourceLayout.Variables = colorVars;
        psoCI.PSODesc.ResourceLayout.NumVariables = _countof(colorVars);
        setupCommonPipeline(psoCI, StencilMode::Equal, true, m_MsaaSamples);
        applyLayerOutputFormats(psoCI);
        psoCI.pVS = m_VS_Color;
        psoCI.pPS = m_PS_Color;
        m_Device->CreateGraphicsPipelineState(psoCI, &m_PSO_Color_StencilEqual);
    }

    {
        Diligent::GraphicsPipelineStateCreateInfo psoCI;
        psoCI.PSODesc.Name = "RmlDiligent_Texture_StencilEqual_PSO";
        psoCI.PSODesc.ResourceLayout.Variables = textureVars;
        psoCI.PSODesc.ResourceLayout.NumVariables = _countof(textureVars);
        setupCommonPipeline(psoCI, StencilMode::Equal, true, m_MsaaSamples);
        applyLayerOutputFormats(psoCI);
        psoCI.pVS = m_VS_Color;
        psoCI.pPS = m_PS_Texture;
        m_Device->CreateGraphicsPipelineState(psoCI, &m_PSO_Texture_StencilEqual);
    }

    {
        Diligent::ShaderResourceVariableDesc shaderCBVarsSE[] = {
            {Diligent::SHADER_TYPE_VERTEX, "ConstantBuffer", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {Diligent::SHADER_TYPE_VERTEX, "ProjectionBuffer", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {Diligent::SHADER_TYPE_PIXEL, "SharedConstantBuffer", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        };
        Diligent::GraphicsPipelineStateCreateInfo psoCI;
        psoCI.PSODesc.Name = "RmlDiligent_Gradient_StencilEqual_PSO";
        psoCI.PSODesc.ResourceLayout.Variables = shaderCBVarsSE;
        psoCI.PSODesc.ResourceLayout.NumVariables = _countof(shaderCBVarsSE);
        setupCommonPipeline(psoCI, StencilMode::Equal, true, m_MsaaSamples);
        psoCI.GraphicsPipeline.pRenderPass = nullptr;
        psoCI.GraphicsPipeline.NumRenderTargets = 1;
        psoCI.GraphicsPipeline.RTVFormats[0] = Diligent::TEX_FORMAT_RGBA8_UNORM;
        psoCI.GraphicsPipeline.DSVFormat = Diligent::TEX_FORMAT_D24_UNORM_S8_UINT;
        psoCI.pVS = m_VS_Color;
        psoCI.pPS = m_PS_Gradient;
        m_Device->CreateGraphicsPipelineState(psoCI, &m_PSO_Gradient_StencilEqual);
    }

    {
        Diligent::ShaderResourceVariableDesc shaderCBVarsSE2[] = {
            {Diligent::SHADER_TYPE_VERTEX, "ConstantBuffer", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {Diligent::SHADER_TYPE_VERTEX, "ProjectionBuffer", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {Diligent::SHADER_TYPE_PIXEL, "SharedConstantBuffer", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        };
        Diligent::GraphicsPipelineStateCreateInfo psoCI;
        psoCI.PSODesc.Name = "RmlDiligent_Creation_StencilEqual_PSO";
        psoCI.PSODesc.ResourceLayout.Variables = shaderCBVarsSE2;
        psoCI.PSODesc.ResourceLayout.NumVariables = _countof(shaderCBVarsSE2);
        setupCommonPipeline(psoCI, StencilMode::Equal, true, m_MsaaSamples);
        psoCI.GraphicsPipeline.pRenderPass = nullptr;
        psoCI.GraphicsPipeline.NumRenderTargets = 1;
        psoCI.GraphicsPipeline.RTVFormats[0] = Diligent::TEX_FORMAT_RGBA8_UNORM;
        psoCI.GraphicsPipeline.DSVFormat = Diligent::TEX_FORMAT_D24_UNORM_S8_UINT;
        psoCI.pVS = m_VS_Color;
        psoCI.pPS = m_PS_Creation;
        m_Device->CreateGraphicsPipelineState(psoCI, &m_PSO_Creation_StencilEqual);
    }

    {
        Diligent::GraphicsPipelineStateCreateInfo psoCI;
        psoCI.PSODesc.Name = "RmlDiligent_Color_StencilSet_PSO";
        psoCI.PSODesc.ResourceLayout.Variables = colorVars;
        psoCI.PSODesc.ResourceLayout.NumVariables = _countof(colorVars);
        setupCommonPipeline(psoCI, StencilMode::MaskReplace, false, m_MsaaSamples);
        applyLayerOutputFormats(psoCI);
        psoCI.pVS = m_VS_Color;
        psoCI.pPS = m_PS_Color;
        m_Device->CreateGraphicsPipelineState(psoCI, &m_PSO_Color_StencilSet);
    }

    {
        Diligent::GraphicsPipelineStateCreateInfo psoCI;
        psoCI.PSODesc.Name = "RmlDiligent_Color_StencilIntersect_PSO";
        psoCI.PSODesc.ResourceLayout.Variables = colorVars;
        psoCI.PSODesc.ResourceLayout.NumVariables = _countof(colorVars);
        setupCommonPipeline(psoCI, StencilMode::MaskIncr, false, m_MsaaSamples);
        applyLayerOutputFormats(psoCI);
        psoCI.pVS = m_VS_Color;
        psoCI.pPS = m_PS_Color;
        m_Device->CreateGraphicsPipelineState(psoCI, &m_PSO_Color_StencilIntersect);
    }

    {
        Diligent::ShaderResourceVariableDesc layerVars[] = {
            {Diligent::SHADER_TYPE_PIXEL, "g_InputTexture", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {Diligent::SHADER_TYPE_PIXEL, "g_SamplerLinear", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        };
        auto setupPassthroughPipeline = [&](Diligent::IPipelineState** ppPSO, const char* name,
                                            Diligent::TEXTURE_FORMAT dsvFormat,
                                            Diligent::TEXTURE_FORMAT rtvFormat = Diligent::TEX_FORMAT_RGBA8_UNORM) {
            Diligent::GraphicsPipelineStateCreateInfo psoCI;
            psoCI.PSODesc.Name = name;
            psoCI.PSODesc.ResourceLayout.Variables = layerVars;
            psoCI.PSODesc.ResourceLayout.NumVariables = _countof(layerVars);
            setupCommonPipeline(psoCI, StencilMode::Off);
            psoCI.GraphicsPipeline.pRenderPass = nullptr;
            psoCI.GraphicsPipeline.NumRenderTargets = 1;
            psoCI.GraphicsPipeline.RTVFormats[0] = rtvFormat;
            psoCI.GraphicsPipeline.DSVFormat = dsvFormat;
            psoCI.pVS = m_VS_PassThrough;
            psoCI.pPS = m_PS_Passthrough;
            m_Device->CreateGraphicsPipelineState(psoCI, ppPSO);
        };

        setupPassthroughPipeline(&m_PSO_Passthrough, "RmlDiligent_Passthrough_PSO", Diligent::TEX_FORMAT_D24_UNORM_S8_UINT);
        setupPassthroughPipeline(&m_PSO_PassthroughPresent, "RmlDiligent_PassthroughPresent_PSO",
                                 Diligent::TEX_FORMAT_UNKNOWN, m_SwapChain->GetDesc().ColorBufferFormat);

        {
            Diligent::GraphicsPipelineStateCreateInfo psoCI;
            psoCI.PSODesc.Name = "RmlDiligent_Passthrough_StencilEqual_PSO";
            psoCI.PSODesc.ResourceLayout.Variables = layerVars;
            psoCI.PSODesc.ResourceLayout.NumVariables = _countof(layerVars);
            setupCommonPipeline(psoCI, StencilMode::Equal);
            psoCI.GraphicsPipeline.pRenderPass = nullptr;
            psoCI.GraphicsPipeline.NumRenderTargets = 1;
            psoCI.GraphicsPipeline.RTVFormats[0] = Diligent::TEX_FORMAT_RGBA8_UNORM;
            psoCI.GraphicsPipeline.DSVFormat = Diligent::TEX_FORMAT_D24_UNORM_S8_UINT;
            psoCI.pVS = m_VS_PassThrough;
            psoCI.pPS = m_PS_Passthrough;
            m_Device->CreateGraphicsPipelineState(psoCI, &m_PSO_Passthrough_StencilEqual);
        }

        {
            Diligent::GraphicsPipelineStateCreateInfo psoCI;
            psoCI.PSODesc.Name = "RmlDiligent_PassthroughOpacity_PSO";
            psoCI.PSODesc.ResourceLayout.Variables = layerVars;
            psoCI.PSODesc.ResourceLayout.NumVariables = _countof(layerVars);
            setupCommonPipeline(psoCI, StencilMode::Off);
            SetupOpacityBlendState(psoCI);
            psoCI.GraphicsPipeline.pRenderPass = nullptr;
            psoCI.GraphicsPipeline.NumRenderTargets = 1;
            psoCI.GraphicsPipeline.RTVFormats[0] = Diligent::TEX_FORMAT_RGBA8_UNORM;
            psoCI.GraphicsPipeline.DSVFormat = Diligent::TEX_FORMAT_UNKNOWN;
            psoCI.pVS = m_VS_PassThrough;
            psoCI.pPS = m_PS_Passthrough;
            m_Device->CreateGraphicsPipelineState(psoCI, &m_PSO_PassthroughOpacity);
        }

        {
            Diligent::GraphicsPipelineStateCreateInfo psoCI;
            psoCI.PSODesc.Name = "RmlDiligent_PassthroughReplace_PSO";
            psoCI.PSODesc.ResourceLayout.Variables = layerVars;
            psoCI.PSODesc.ResourceLayout.NumVariables = _countof(layerVars);
            setupCommonPipeline(psoCI, StencilMode::Off);
            SetupNoBlendState(psoCI);
            psoCI.GraphicsPipeline.pRenderPass = nullptr;
            psoCI.GraphicsPipeline.NumRenderTargets = 1;
            psoCI.GraphicsPipeline.RTVFormats[0] = Diligent::TEX_FORMAT_RGBA8_UNORM;
            psoCI.GraphicsPipeline.DSVFormat = Diligent::TEX_FORMAT_UNKNOWN;
            psoCI.pVS = m_VS_PassThrough;
            psoCI.pPS = m_PS_Passthrough;
            m_Device->CreateGraphicsPipelineState(psoCI, &m_PSO_PassthroughReplace);
        }
    }

    {
        Diligent::ShaderResourceVariableDesc blurVars[] = {
            {Diligent::SHADER_TYPE_PIXEL, "g_InputTexture", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {Diligent::SHADER_TYPE_PIXEL, "g_SamplerLinear", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {Diligent::SHADER_TYPE_VERTEX, "SharedConstantBuffer", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {Diligent::SHADER_TYPE_PIXEL, "SharedConstantBuffer", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        };
        Diligent::GraphicsPipelineStateCreateInfo psoCI;
        psoCI.PSODesc.Name = "RmlDiligent_Blur_PSO";
        psoCI.PSODesc.ResourceLayout.Variables = blurVars;
        psoCI.PSODesc.ResourceLayout.NumVariables = _countof(blurVars);
        setupCommonPipeline(psoCI, StencilMode::Off);
        SetupNoBlendState(psoCI);
        psoCI.GraphicsPipeline.pRenderPass = nullptr;
        psoCI.GraphicsPipeline.NumRenderTargets = 1;
        psoCI.GraphicsPipeline.RTVFormats[0] = Diligent::TEX_FORMAT_RGBA8_UNORM;
        psoCI.GraphicsPipeline.DSVFormat = Diligent::TEX_FORMAT_UNKNOWN;
        psoCI.pVS = m_VS_Blur;
        psoCI.pPS = m_PS_Blur;
        m_Device->CreateGraphicsPipelineState(psoCI, &m_PSO_Blur);
    }

    {
        Diligent::ShaderResourceVariableDesc dropShadowVars[] = {
            {Diligent::SHADER_TYPE_PIXEL, "g_InputTexture", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {Diligent::SHADER_TYPE_PIXEL, "g_SamplerLinear", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {Diligent::SHADER_TYPE_PIXEL, "DropShadowBuffer", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        };
        Diligent::GraphicsPipelineStateCreateInfo psoCI;
        psoCI.PSODesc.Name = "RmlDiligent_DropShadow_PSO";
        psoCI.PSODesc.ResourceLayout.Variables = dropShadowVars;
        psoCI.PSODesc.ResourceLayout.NumVariables = _countof(dropShadowVars);
        setupCommonPipeline(psoCI, StencilMode::Off);
        SetupNoBlendState(psoCI);
        psoCI.GraphicsPipeline.pRenderPass = nullptr;
        psoCI.GraphicsPipeline.NumRenderTargets = 1;
        psoCI.GraphicsPipeline.RTVFormats[0] = Diligent::TEX_FORMAT_RGBA8_UNORM;
        psoCI.GraphicsPipeline.DSVFormat = Diligent::TEX_FORMAT_UNKNOWN;
        psoCI.pVS = m_VS_PassThrough;
        psoCI.pPS = m_PS_DropShadow;
        m_Device->CreateGraphicsPipelineState(psoCI, &m_PSO_DropShadow);
    }

    {
        Diligent::ShaderResourceVariableDesc colorMatrixVars[] = {
            {Diligent::SHADER_TYPE_PIXEL, "g_InputTexture", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {Diligent::SHADER_TYPE_PIXEL, "g_SamplerLinear", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {Diligent::SHADER_TYPE_PIXEL, "ConstantBuffer", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        };
        Diligent::GraphicsPipelineStateCreateInfo psoCI;
        psoCI.PSODesc.Name = "RmlDiligent_ColorMatrix_PSO";
        psoCI.PSODesc.ResourceLayout.Variables = colorMatrixVars;
        psoCI.PSODesc.ResourceLayout.NumVariables = _countof(colorMatrixVars);
        setupCommonPipeline(psoCI, StencilMode::Off);
        SetupNoBlendState(psoCI);
        psoCI.GraphicsPipeline.pRenderPass = nullptr;
        psoCI.GraphicsPipeline.NumRenderTargets = 1;
        psoCI.GraphicsPipeline.RTVFormats[0] = Diligent::TEX_FORMAT_RGBA8_UNORM;
        psoCI.GraphicsPipeline.DSVFormat = Diligent::TEX_FORMAT_UNKNOWN;
        psoCI.pVS = m_VS_PassThrough;
        psoCI.pPS = m_PS_ColorMatrix;
        m_Device->CreateGraphicsPipelineState(psoCI, &m_PSO_ColorMatrix);
    }

    {
        Diligent::ShaderResourceVariableDesc blendMaskVars[] = {
            {Diligent::SHADER_TYPE_PIXEL, "g_InputTexture", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {Diligent::SHADER_TYPE_PIXEL, "g_MaskTexture", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {Diligent::SHADER_TYPE_PIXEL, "g_SamplerLinear", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        };
        Diligent::GraphicsPipelineStateCreateInfo psoCI;
        psoCI.PSODesc.Name = "RmlDiligent_BlendMask_PSO";
        psoCI.PSODesc.ResourceLayout.Variables = blendMaskVars;
        psoCI.PSODesc.ResourceLayout.NumVariables = _countof(blendMaskVars);
        setupCommonPipeline(psoCI, StencilMode::Off);
        SetupNoBlendState(psoCI);
        psoCI.GraphicsPipeline.pRenderPass = nullptr;
        psoCI.GraphicsPipeline.NumRenderTargets = 1;
        psoCI.GraphicsPipeline.RTVFormats[0] = Diligent::TEX_FORMAT_RGBA8_UNORM;
        psoCI.GraphicsPipeline.DSVFormat = Diligent::TEX_FORMAT_UNKNOWN;
        psoCI.pVS = m_VS_PassThrough;
        psoCI.pPS = m_PS_BlendMask;
        m_Device->CreateGraphicsPipelineState(psoCI, &m_PSO_BlendMask);
    }

    {
        // MSAA composite PSO：从 MSAA layer RT resolve 到 postprocess RT 时使用
        Diligent::ShaderResourceVariableDesc compositeVars[] = {
            {Diligent::SHADER_TYPE_PIXEL, "g_InputTexture", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {Diligent::SHADER_TYPE_PIXEL, "g_SamplerLinear", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        };
        {
            Diligent::GraphicsPipelineStateCreateInfo psoCI;
            psoCI.PSODesc.Name = "RmlDiligent_Composite_PSO";
            psoCI.PSODesc.ResourceLayout.Variables = compositeVars;
            psoCI.PSODesc.ResourceLayout.NumVariables = _countof(compositeVars);
            setupCommonPipeline(psoCI, StencilMode::Off);
            psoCI.GraphicsPipeline.pRenderPass = nullptr;
            psoCI.GraphicsPipeline.NumRenderTargets = 1;
            psoCI.GraphicsPipeline.RTVFormats[0] = Diligent::TEX_FORMAT_RGBA8_UNORM;
            psoCI.GraphicsPipeline.DSVFormat = Diligent::TEX_FORMAT_D24_UNORM_S8_UINT;
            psoCI.pVS = m_VS_PassThrough;
            psoCI.pPS = m_PS_Passthrough;
            m_Device->CreateGraphicsPipelineState(psoCI, &m_PSO_Composite);
        }
        {
            Diligent::GraphicsPipelineStateCreateInfo psoCI;
            psoCI.PSODesc.Name = "RmlDiligent_CompositeReplace_PSO";
            psoCI.PSODesc.ResourceLayout.Variables = compositeVars;
            psoCI.PSODesc.ResourceLayout.NumVariables = _countof(compositeVars);
            setupCommonPipeline(psoCI, StencilMode::Off);
            SetupNoBlendState(psoCI);
            psoCI.GraphicsPipeline.pRenderPass = nullptr;
            psoCI.GraphicsPipeline.NumRenderTargets = 1;
            psoCI.GraphicsPipeline.RTVFormats[0] = Diligent::TEX_FORMAT_RGBA8_UNORM;
            psoCI.GraphicsPipeline.DSVFormat = Diligent::TEX_FORMAT_UNKNOWN;
            psoCI.pVS = m_VS_PassThrough;
            psoCI.pPS = m_PS_Passthrough;
            m_Device->CreateGraphicsPipelineState(psoCI, &m_PSO_CompositeReplace);
        }
        // MSAA + StencilEqual：backdrop-filter 圆角裁剪、box-shadow stencil 保护
        {
            Diligent::GraphicsPipelineStateCreateInfo psoCI;
            psoCI.PSODesc.Name = "RmlDiligent_Composite_StencilEqual_PSO";
            psoCI.PSODesc.ResourceLayout.Variables = compositeVars;
            psoCI.PSODesc.ResourceLayout.NumVariables = _countof(compositeVars);
            setupCommonPipeline(psoCI, StencilMode::Equal, true, m_MsaaSamples);
            psoCI.GraphicsPipeline.pRenderPass = nullptr;
            psoCI.GraphicsPipeline.NumRenderTargets = 1;
            psoCI.GraphicsPipeline.RTVFormats[0] = Diligent::TEX_FORMAT_RGBA8_UNORM;
            psoCI.GraphicsPipeline.DSVFormat = Diligent::TEX_FORMAT_D24_UNORM_S8_UINT;
            psoCI.pVS = m_VS_PassThrough;
            psoCI.pPS = m_PS_Passthrough;
            m_Device->CreateGraphicsPipelineState(psoCI, &m_PSO_Composite_StencilEqual);
        }
    }

    {
        // UV layout matches D3D RT (0,0)=top-left: screen-top samples v=0, screen-bottom samples v=1.
        const Rml::ColourbPremultiplied white(255, 255, 255, 255);
        Rml::Vertex vertices[4] = {
            {{-1.0f, -1.0f}, white, {0.0f, 1.0f}},
            {{1.0f, -1.0f}, white, {1.0f, 1.0f}},
            {{1.0f, 1.0f}, white, {1.0f, 0.0f}},
            {{-1.0f, 1.0f}, white, {0.0f, 0.0f}},
        };
        const int indices[6] = {0, 1, 2, 0, 2, 3};

        Diligent::BufferDesc vbDesc;
        vbDesc.Size = sizeof(vertices);
        vbDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
        vbDesc.Usage = Diligent::USAGE_DYNAMIC;
        vbDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
        m_Device->CreateBuffer(vbDesc, nullptr, &m_FullscreenVB);
        if (m_FullscreenVB) {
            Diligent::MapHelper<Rml::Vertex> mapped(
                m_Context, m_FullscreenVB, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
            if (mapped) {
                memcpy(mapped, vertices, sizeof(vertices));
            }
        }

        Diligent::BufferDesc ibDesc;
        ibDesc.Size = sizeof(indices);
        ibDesc.BindFlags = Diligent::BIND_INDEX_BUFFER;
        ibDesc.Usage = Diligent::USAGE_IMMUTABLE;
        Diligent::BufferData ibData(indices, ibDesc.Size);
        m_Device->CreateBuffer(ibDesc, &ibData, &m_FullscreenIB);
    }
}

// =============================================================================
// 创建动态 CB
// =============================================================================

void RmlDiligentRenderInterface::CreateConstantBuffer()
{
    Diligent::BufferDesc cbDesc;
    cbDesc.Name = "RmlDiligent CB";
    cbDesc.Size = (sizeof(MainCB) + 255) & ~255u;  // D3D12 CB 需 256 字节对齐
    cbDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
    cbDesc.Usage = Diligent::USAGE_DYNAMIC;
    cbDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;

    m_Device->CreateBuffer(cbDesc, nullptr, &m_ConstantBuffer);
}

void RmlDiligentRenderInterface::CreateShaderConstantBuffers()
{
    {
        Diligent::BufferDesc cbDesc;
        cbDesc.Name = "RmlDiligent Projection CB";
        cbDesc.Size = (sizeof(float) * 16 + 255) & ~255u; // 64 bytes, 256-aligned
        cbDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
        cbDesc.Usage = Diligent::USAGE_DYNAMIC;
        cbDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
        m_Device->CreateBuffer(cbDesc, nullptr, &m_ProjectionCB);
    }
    {
        Diligent::BufferDesc cbDesc;
        cbDesc.Name = "RmlDiligent Gradient CB";
        cbDesc.Size = (sizeof(GradientCB) + 255) & ~255u;
        cbDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
        cbDesc.Usage = Diligent::USAGE_DYNAMIC;
        cbDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
        m_Device->CreateBuffer(cbDesc, nullptr, &m_GradientCB);
    }
    {
        Diligent::BufferDesc cbDesc;
        cbDesc.Name = "RmlDiligent Creation CB";
        cbDesc.Size = (sizeof(CreationCB) + 255) & ~255u;
        cbDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
        cbDesc.Usage = Diligent::USAGE_DYNAMIC;
        cbDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
        m_Device->CreateBuffer(cbDesc, nullptr, &m_CreationCB);
    }
    {
        Diligent::BufferDesc cbDesc;
        cbDesc.Name = "RmlDiligent Blur CB";
        cbDesc.Size = (sizeof(BlurCB) + 255) & ~255u;
        cbDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
        cbDesc.Usage = Diligent::USAGE_DYNAMIC;
        cbDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
        m_Device->CreateBuffer(cbDesc, nullptr, &m_BlurCB);
    }
    {
        Diligent::BufferDesc cbDesc;
        cbDesc.Name = "RmlDiligent DropShadow CB";
        cbDesc.Size = (sizeof(DropShadowCBData) + 255) & ~255u;
        cbDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
        cbDesc.Usage = Diligent::USAGE_DYNAMIC;
        cbDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
        m_Device->CreateBuffer(cbDesc, nullptr, &m_DropShadowCB);
    }
    {
        Diligent::BufferDesc cbDesc;
        cbDesc.Name = "RmlDiligent ColorMatrix CB";
        cbDesc.Size = (sizeof(ColorMatrixCBData) + 255) & ~255u;
        cbDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
        cbDesc.Usage = Diligent::USAGE_DYNAMIC;
        cbDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
        m_Device->CreateBuffer(cbDesc, nullptr, &m_ColorMatrixCB);
    }
}

void RmlDiligentRenderInterface::DrawIndexedGeometry(GeometryHandle* geom)
{
    if (!geom || !geom->vbAlloc.buffer || !geom->ibAlloc.buffer || geom->indexCount == 0) {
        return;
    }

#ifdef RML_PERF_COUNTERS
    LARGE_INTEGER vbibT0, vbibT1;
    QueryPerformanceCounter(&vbibT0);
#endif
    // 使用 BufferManager 子分配的共享 buffer + offset
    Diligent::Uint64 vbOffset = geom->vbAlloc.offset;
    Diligent::IBuffer* pVB = geom->vbAlloc.buffer;
    m_Context->SetVertexBuffers(0, 1, &pVB, &vbOffset, Diligent::RESOURCE_STATE_TRANSITION_MODE_NONE, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
    m_Context->SetIndexBuffer(geom->ibAlloc.buffer, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_NONE);
#ifdef RML_PERF_COUNTERS
    QueryPerformanceCounter(&vbibT1);
    m_PerfCounters.setVBIBUs += vbibT1.QuadPart - vbibT0.QuadPart;
#endif

    const Rml::Matrix4f& identity = Rml::Matrix4f::Identity();
    const bool hasTransform = std::memcmp(m_Transform.data(), identity.data(), sizeof(float) * 16) != 0;
    const bool useTransformedDraw = hasTransform && m_ScissorEnabled;

#ifdef RML_PERF_COUNTERS
    LARGE_INTEGER scT0, scT1;
    QueryPerformanceCounter(&scT0);
#endif
    if (useTransformedDraw) {
        if (m_Context && m_SwapChain) {
            const int w = static_cast<int>(m_SwapChain->GetDesc().Width);
            const int h = static_cast<int>(m_SwapChain->GetDesc().Height);
            Diligent::Rect fullScissor{};
            fullScissor.left = 0;
            fullScissor.top = 0;
            fullScissor.right = w;
            fullScissor.bottom = h;
            m_Context->SetScissorRects(1, &fullScissor, static_cast<Diligent::Uint32>(w), static_cast<Diligent::Uint32>(h));
        }
    } else if (m_ScissorEnabled && m_ScissorRegionRml.Valid()) {
        SetScissorRml(m_ScissorRegionRml, false);
    } else if (m_Context && m_SwapChain) {
        const int w = static_cast<int>(m_SwapChain->GetDesc().Width);
        const int h = static_cast<int>(m_SwapChain->GetDesc().Height);
        Diligent::Rect fullScissor{};
        fullScissor.left = 0;
        fullScissor.top = 0;
        fullScissor.right = w;
        fullScissor.bottom = h;
        m_Context->SetScissorRects(1, &fullScissor, static_cast<Diligent::Uint32>(w), static_cast<Diligent::Uint32>(h));
    }
#ifdef RML_PERF_COUNTERS
    QueryPerformanceCounter(&scT1);
    m_PerfCounters.setScissorUs += scT1.QuadPart - scT0.QuadPart;
#endif

    // 诊断：绘制 transform 元素时的状态
    static uint32_t s_drawDiag = 0;
    if (hasTransform && s_drawDiag < 30) {
        ++s_drawDiag;
        const int sw = m_SwapChain ? static_cast<int>(m_SwapChain->GetDesc().Width) : 0;
        const int sh = m_SwapChain ? static_cast<int>(m_SwapChain->GetDesc().Height) : 0;
        BoxShadowDiagLog(
            "[GPU] hasT=%d useT=%d scissorOn=%d clip=%d stencil=%d | "
            "scissorRml=%d,%d,%d,%d | swapchain=%dx%d | idx=%u",
            (int)hasTransform, (int)useTransformedDraw, (int)m_ScissorEnabled,
            (int)m_ClipMaskEnabled, (int)m_StencilTestValue,
            m_ScissorRegionRml.Left(), m_ScissorRegionRml.Top(),
            m_ScissorRegionRml.Right(), m_ScissorRegionRml.Bottom(),
            sw, sh, geom->indexCount);
    }

    Diligent::DrawIndexedAttribs drawAttrs(geom->indexCount, Diligent::VT_UINT32, Diligent::DRAW_FLAG_NONE);
#ifdef RML_PERF_COUNTERS
    LARGE_INTEGER drawT0, drawT1;
    QueryPerformanceCounter(&drawT0);
#endif
    m_Context->DrawIndexed(drawAttrs);
#ifdef RML_PERF_COUNTERS
    QueryPerformanceCounter(&drawT1);
    ++m_PerfCounters.drawIndexed;
    m_PerfCounters.drawIndexedUs += drawT1.QuadPart - drawT0.QuadPart;
#endif

    if (useTransformedDraw && m_ScissorEnabled && m_ScissorRegionRml.Valid()) {
#ifdef RML_PERF_COUNTERS
        LARGE_INTEGER scT2, scT3;
        QueryPerformanceCounter(&scT2);
#endif
        SetScissorRml(m_ScissorRegionRml, false);
#ifdef RML_PERF_COUNTERS
        QueryPerformanceCounter(&scT3);
        m_PerfCounters.setScissorUs += scT3.QuadPart - scT2.QuadPart;
#endif
    }
}

void RmlDiligentRenderInterface::EnsureFramebufferBound()
{
    if (m_SwapchainPassActive) {
        return;
    }
    if (m_LayerStack.GetLayerCount() > 0) {
        BindTopLayer();
    } else if (m_OffscreenRTV) {
        // 离屏模式：重新绑定 offscreen RT（防止 Render 过程中状态被重置）
        Diligent::ITextureView* RTVs[] = { m_OffscreenRTV };
        m_Context->SetRenderTargets(1, RTVs, m_OffscreenDSV, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
    }
}

// =============================================================================
// RenderStateCache：BindPSO / BindSRB（脏检测，过滤冗余驱动调用）
// =============================================================================

void RmlDiligentRenderInterface::BindPSO(Diligent::IPipelineState* pso)
{
#ifdef RML_PERF_COUNTERS
    ++m_PerfCounters.setPSOReq;
    LARGE_INTEGER t0, t1;
    QueryPerformanceCounter(&t0);
#endif
    if (m_StateCache.currentPSO == pso) {
        return;
    }
    m_Context->SetPipelineState(pso);
    m_StateCache.currentPSO = pso;
    m_StateCache.currentSRB = nullptr;  // PSO 变了 → SRB 失效（implicit signature 不同）
#ifdef RML_PERF_COUNTERS
    ++m_PerfCounters.setPSOReal;
    QueryPerformanceCounter(&t1);
    m_PerfCounters.bindPSOUs += t1.QuadPart - t0.QuadPart;
#endif
}

void RmlDiligentRenderInterface::BindSRB(
    Diligent::IShaderResourceBinding* srb,
    Diligent::RESOURCE_STATE_TRANSITION_MODE mode)
{
#ifdef RML_PERF_COUNTERS
    ++m_PerfCounters.commitSRBReq;
    LARGE_INTEGER t0, t1;
    QueryPerformanceCounter(&t0);
#endif
    if (m_StateCache.currentSRB == srb) {
        return;
    }
    m_Context->CommitShaderResources(srb, mode);
    m_StateCache.currentSRB = srb;
#ifdef RML_PERF_COUNTERS
    ++m_PerfCounters.commitSRBReal;
    QueryPerformanceCounter(&t1);
    m_PerfCounters.bindSRBUs += t1.QuadPart - t0.QuadPart;
#endif
}

void RmlDiligentRenderInterface::DrawColorGeometry(
    GeometryHandle* geom,
    Rml::Vector2f translation,
    Diligent::IPipelineState* pso)
{
    if (!geom || !pso || !m_ConstantBuffer) {
        return;
    }

    BindPSO(pso);

    {
#ifdef RML_PERF_COUNTERS
        LARGE_INTEGER mapT0, mapT1;
        QueryPerformanceCounter(&mapT0);
#endif
        Diligent::MapHelper<MainCB> cb(m_Context, m_ConstantBuffer, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
        UploadTransformToCB(cb, m_Transform, translation);
#ifdef RML_PERF_COUNTERS
        QueryPerformanceCounter(&mapT1);
        ++m_PerfCounters.mapCB;
        m_PerfCounters.mapCBUs += mapT1.QuadPart - mapT0.QuadPart;
#endif
    }

    // SRB 必须匹配 PSO 的 implicit signature
    Diligent::IShaderResourceBinding* srb = (pso == m_PSO_Color_StencilEqual) ? m_SRB_Color_StencilEqual : m_SRB_Color;
    if (srb) {
        BindSRB(srb);
    }

    DrawIndexedGeometry(geom);
}

// =============================================================================
// 设置投影矩阵
// =============================================================================

void RmlDiligentRenderInterface::SetProjectionMatrix(int width, int height)
{
    ClearSrbCachesOnResize(width, height);
    // Diligent (D3D12) 使用 Z ∈ [0, 1]，而 RmlUi 的 ProjectOrtho 输出 Z ∈ [-1,1]（OpenGL 约定）。
    // 3D 旋转后 Z 变负 → GPU 裁剪 → 只显示半边。
    // 手动构建 D3D12 约定的正交投影矩阵：Z 映射到 [0, 1]。
    const float l = 0.f, r = static_cast<float>(width);
    const float b = static_cast<float>(height), t = 0.f;
    const float n = -10000.f, f = 10000.f;
    Rml::Matrix4f proj = Rml::Matrix4f::Identity();
    float* d = proj.data();
    d[0]  = 2.f / (r - l);           // [0][0]
    d[5]  = 2.f / (t - b);           // [1][1]
    d[10] = 1.f / (f - n);           // [2][2] — D3D12: 1/(f-n)，不是 2/(f-n)
    d[12] = -(r + l) / (r - l);      // [3][0]
    d[13] = -(t + b) / (t - b);      // [3][1]
    d[14] = -n / (f - n);            // [3][2] — D3D12: -n/(f-n)，不是 -(f+n)/(f-n)
    d[15] = 1.f;                     // [3][3]
    m_Projection = proj;
    m_Transform = Rml::Matrix4f::Identity();
    m_LayerStack.OnResize(width, height);
}

void RmlDiligentRenderInterface::ClearSrbCachesOnResize(int width, int height)
{
    if (width == m_CachedWidth && height == m_CachedHeight) {
        return;
    }
    m_CachedWidth = width;
    m_CachedHeight = height;
    // RT Pool 重建后 layer/postprocess SRV 指针变化；纹理级 SRB 仍有效（同一 Texture 对象）。
    m_SrbCache.Clear();
}

MemoryStats RmlDiligentRenderInterface::GetMemoryStats() const
{
    MemoryStats stats{};
    stats.pooledRtFreeCount = m_RTPool.GetFreeListSize();
    stats.pooledRtActiveLayers = m_RTPool.GetActiveAcquireCount();
    stats.srbCacheEntries = m_SrbCache.GetEntryCount();
    stats.textureSrbEntries = m_TextureSrbEntryCount;
    return stats;
}

void RmlDiligentRenderInterface::ResetPerfStats()
{
    m_SrbCache.ResetStats();
    ResetDrawCount();
    m_CompositeCount = 0;
    m_FilterRenderCount = 0;
    m_PushLayerCount = 0;
}

Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> RmlDiligentRenderInterface::GetOrCreateTextureSRBRef(
    TextureHandle* tex,
    ProgramId id,
    Diligent::IPipelineState* pso)
{
    if (!tex || !pso) {
        return {};
    }

    if (m_SrbCacheEnabled) {
        if (auto it = tex->srbCache.find(id); it != tex->srbCache.end() && it->second) {
            return it->second;
        }
    }

    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> srb;
    pso->CreateShaderResourceBinding(&srb, true);
    if (!srb) {
        return {};
    }

    if (auto* cbVar = srb->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, "ConstantBuffer")) {
        cbVar->Set(m_ConstantBuffer);
    }
    if (auto* projVar = srb->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, "ProjectionBuffer")) {
        projVar->Set(m_ProjectionCB);
    }
    if (auto* texVar = srb->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_InputTexture")) {
        texVar->Set(tex->SRV);
    }
    if (auto* sampVar = srb->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_SamplerLinear")) {
        sampVar->Set(m_Sampler);
    }

    if (m_SrbCacheEnabled) {
        tex->srbCache[id] = srb;
        ++m_TextureSrbEntryCount;
    }
    return srb;
}

Diligent::IShaderResourceBinding* RmlDiligentRenderInterface::GetOrCreateTextureSRB(
    TextureHandle* tex,
    ProgramId id,
    Diligent::IPipelineState* pso)
{
    return GetOrCreateTextureSRBRef(tex, id, pso).RawPtr();
}

Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> RmlDiligentRenderInterface::GetOrCreateCachedSRB(
    Diligent::IPipelineState* pso,
    Diligent::ITextureView* primarySrv,
    Diligent::IBuffer* constantBuffer,
    const SrbCache::BindFn& bindFn,
    Diligent::ITextureView* secondarySrv)
{
    return m_SrbCache.GetOrCreate(pso, primarySrv, constantBuffer, m_SrbCacheEnabled, bindFn, secondarySrv);
}

Diligent::ITextureView* RmlDiligentRenderInterface::GetActiveDepthStencilDSV() const
{
    return m_LayerStack.GetSharedDepthDSV();
}

void RmlDiligentRenderInterface::BindLayer(Rml::LayerHandle layer)
{
    const auto& layerRT = m_LayerStack.GetLayer(layer);
    if (!layerRT.RTV) {
        return;
    }

    Diligent::ITextureView* pRTV = layerRT.RTV.RawPtr();
    Diligent::ITextureView* pDSV = GetActiveDepthStencilDSV();
    m_Context->SetRenderTargets(1, &pRTV, pDSV, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
}

void RmlDiligentRenderInterface::BindTopLayer()
{
    BindLayer(m_LayerStack.GetTopLayerHandle());
}

void RmlDiligentRenderInterface::UnbindRenderTargets()
{
    m_Context->SetRenderTargets(0, nullptr, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
}

// =============================================================================
// 开始/结束帧
// =============================================================================

void RmlDiligentRenderInterface::BeginFrame()
{
#ifdef RML_PERF_COUNTERS
    LARGE_INTEGER tick0;
    QueryPerformanceCounter(&tick0);
#endif

    // 每帧重置所有性能计数器
    ResetPerfStats();
    m_SaveLayerAsTextureCount = 0;
    m_CallbackTextureDrawCount = 0;
    m_StateCache.Reset();
#ifdef RML_PERF_COUNTERS
    m_PerfCounters = {};
    m_PerfCounters.frameStartTick = tick0.QuadPart;
#endif

    const auto w = static_cast<int>(m_SwapChain->GetDesc().Width);
    const auto h = static_cast<int>(m_SwapChain->GetDesc().Height);

    m_SwapchainPassActive = false;
    m_ClipMaskEnabled = false;
    m_UseStencilEqual = false;
    m_StencilTestValue = 0;
    EnableClipMask(false);

    // 每帧上传投影矩阵（动态缓冲区必须每帧映射）
    if (m_ProjectionCB && m_Context) {
        Diligent::MapHelper<float> cb(m_Context, m_ProjectionCB, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
        if (cb) {
            memcpy(cb, m_Projection.data(), sizeof(float) * 16);
        }
    }

    m_LayerStack.BeginFrame(w, h);
    BindTopLayer();

    Diligent::OptimizedClearValue clearVal{};
    clearVal.Color[0] = 0.0f;
    clearVal.Color[1] = 0.0f;
    clearVal.Color[2] = 0.0f;
    clearVal.Color[3] = 0.0f;
    const auto& topLayer = m_LayerStack.GetTopLayer();
    if (auto* pRTV = topLayer.RTV.RawPtr()) {
        m_Context->ClearRenderTarget(pRTV, &clearVal, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    }
    if (auto* pDSV = GetActiveDepthStencilDSV()) {
        m_Context->ClearDepthStencil(pDSV, Diligent::CLEAR_STENCIL_FLAG, 0.0f, 0,
            Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    }

    // 清空 postprocess buffers，防止 pool 复用纹理时残留上一帧/上一用途的内容。
    // GL3 每帧重建 FBO（内容天然清空），Diligent 复用 pool 纹理需要显式清空。
    {
        auto& pp0 = m_LayerStack.GetPostprocessPrimary();
        auto& pp1 = m_LayerStack.GetPostprocessSecondary();
        Diligent::OptimizedClearValue zero{};
        zero.Color[0] = zero.Color[1] = zero.Color[2] = zero.Color[3] = 0.0f;
        if (auto* rtv0 = pp0.RTV.RawPtr()) {
            m_Context->SetRenderTargets(1, &rtv0, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
            m_Context->ClearRenderTarget(rtv0, &zero, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        }
        if (auto* rtv1 = pp1.RTV.RawPtr()) {
            m_Context->SetRenderTargets(1, &rtv1, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
            m_Context->ClearRenderTarget(rtv1, &zero, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        }
    }

    // 清空 postprocess buffers 后重新绑定根 layer 为渲染目标
    BindTopLayer();

    Diligent::Viewport viewport;
    viewport.Width = static_cast<float>(w);
    viewport.Height = static_cast<float>(h);
    viewport.MaxDepth = 1.0f;
    m_Context->SetViewports(1, &viewport, 0, 0);

    m_ScissorEnabled = false;
    SetTransform(nullptr);

#ifdef RML_PERF_COUNTERS
    LARGE_INTEGER tick1;
    QueryPerformanceCounter(&tick1);
    m_PerfCounters.beginFrameEndTick = tick1.QuadPart;
    m_PerfCounters.beginFrameUs = tick1.QuadPart - tick0.QuadPart;
#endif
}

void RmlDiligentRenderInterface::EndFrame()
{
#ifdef RML_PERF_COUNTERS
    LARGE_INTEGER endFrameTick0;
    QueryPerformanceCounter(&endFrameTick0);
    m_PerfCounters.endFrameStartTick = endFrameTick0.QuadPart;
#endif

    if (m_SwapchainPassActive) {
        m_Context->EndRenderPass();
        m_SwapchainPassActive = false;
    }

    BlitLayerToPostprocessPrimary(m_LayerStack.GetTopLayerHandle());

    auto* pRTV = m_SwapChain->GetCurrentBackBufferRTV();
    Diligent::OptimizedClearValue clearVal{};
    clearVal.Color[0] = 0.18f;
    clearVal.Color[1] = 0.18f;
    clearVal.Color[2] = 0.18f;
    clearVal.Color[3] = 1.0f;
    m_Context->SetRenderTargets(1, &pRTV, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    m_Context->ClearRenderTarget(pRTV, &clearVal, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    auto& postPrimary = m_LayerStack.GetPostprocessPrimary();
    SetSwapchainViewport();
    const int w = static_cast<int>(m_SwapChain->GetDesc().Width);
    const int h = static_cast<int>(m_SwapChain->GetDesc().Height);
    SetScissorRml(Rml::Rectanglei::FromCorners(Rml::Vector2i(0, 0), Rml::Vector2i(w, h)));
    DrawFullscreenPassthrough(postPrimary.SRV.RawPtr(), pRTV, Rml::BlendMode::Blend, false);

    // Match RmlUi_Renderer_DX12::EndFrame: restore postprocess RT state after sampling, keep swapchain bound for Present.
    if (auto* pPostRTV = postPrimary.RTV.RawPtr()) {
        m_Context->SetRenderTargets(1, &pPostRTV, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    }
    m_Context->SetRenderTargets(1, &pRTV, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    m_LayerStack.EndFrame();

    static uint32_t s_DiagFrame = 0;
    ++s_DiagFrame;
    if (m_SaveLayerAsTextureCount > 0 || m_CallbackTextureDrawCount > 0 || s_DiagFrame <= 5 || s_DiagFrame % 120 == 0) {
        BoxShadowDiagLog(
            "[boxshadow-diag] frame=%u SaveLayer=%u CallbackDraw=%u textureDraws=%u push=%u composite=%u filter=%u",
            s_DiagFrame,
            m_SaveLayerAsTextureCount,
            m_CallbackTextureDrawCount,
            m_TextureDrawCount,
            m_PushLayerCount,
            m_CompositeCount,
            m_FilterRenderCount);
    }

#ifdef RML_PERF_COUNTERS
    // EndFrame 出口计时
    LARGE_INTEGER endFrameTick1;
    QueryPerformanceCounter(&endFrameTick1);
    m_PerfCounters.endFrameUs = endFrameTick1.QuadPart - m_PerfCounters.endFrameStartTick;

    // Present 耗时（由 MarkPresentStart/End 记录）
    if (m_PerfCounters.presentStartTick > 0 && m_PerfCounters.presentEndTick > 0) {
        m_PerfCounters.presentUs = m_PerfCounters.presentEndTick - m_PerfCounters.presentStartTick;
    }

    // 整帧耗时：BeginFrame 入口 → Present 出口（或 EndFrame 出口）
    const int64_t frameEndTick = (m_PerfCounters.presentEndTick > 0)
        ? m_PerfCounters.presentEndTick : endFrameTick1.QuadPart;
    m_PerfCounters.frameTotalUs = frameEndTick - m_PerfCounters.frameStartTick;

    // RmlUi CPU 侧耗时
    if (m_PerfCounters.updateStartTick > 0 && m_PerfCounters.updateEndTick > 0) {
        m_PerfCounters.rmlUpdateUs = m_PerfCounters.updateEndTick - m_PerfCounters.updateStartTick;
    }
    if (m_PerfCounters.renderStartTick > 0 && m_PerfCounters.renderEndTick > 0) {
        m_PerfCounters.rmlRenderUs = m_PerfCounters.renderEndTick - m_PerfCounters.renderStartTick;
    }
    // GPU 同步耗时
    if (m_PerfCounters.flushStartTick > 0 && m_PerfCounters.flushEndTick > 0) {
        m_PerfCounters.flushUs = m_PerfCounters.flushEndTick - m_PerfCounters.flushStartTick;
    }
    if (m_PerfCounters.idleGPUStartTick > 0 && m_PerfCounters.idleGPUEndTick > 0) {
        m_PerfCounters.idleGPUUs = m_PerfCounters.idleGPUEndTick - m_PerfCounters.idleGPUStartTick;
    }

    // 转换为毫秒（使用 QPC 频率）
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    auto toMs = [&](int64_t ticks) -> double {
        return 1000.0 * static_cast<double>(ticks) / static_cast<double>(freq.QuadPart);
    };

    // 每 120 帧输出一次性能计数器统计（DebugView / OutputDebugString）
    if (s_DiagFrame % 30 == 0) {
        // CompileGeometry 每帧增量（而非累计值）
        const uint32_t dCompile = m_PerfCounters.compileGeometry - m_PerfCounters.prevCompileGeometry;
        const int64_t dCompileUs = m_PerfCounters.compileGeometryUs - m_PerfCounters.prevCompileGeometryUs;
        const double msCompileDelta = toMs(dCompileUs);
        const double msCompileTotal = toMs(m_PerfCounters.compileGeometryUs);
        m_PerfCounters.prevCompileGeometry = m_PerfCounters.compileGeometry;
        m_PerfCounters.prevCompileGeometryUs = m_PerfCounters.compileGeometryUs;

        const double msRelease = toMs(m_PerfCounters.releaseGeometryUs);
        const double msPSO = toMs(m_PerfCounters.bindPSOUs);
        const double msSRB = toMs(m_PerfCounters.bindSRBUs);
        const double msMap = toMs(m_PerfCounters.mapCBUs);
        const double msDraw = toMs(m_PerfCounters.drawIndexedUs);
        const double msScissor = toMs(m_PerfCounters.setScissorUs);
        const double msVBIB = toMs(m_PerfCounters.setVBIBUs);
        const double msStencil = toMs(m_PerfCounters.setStencilRefUs);
        const double msSrbLookup = toMs(m_PerfCounters.srbLookupUs);
        const double msSum = msPSO + msSRB + msMap + msDraw + msScissor + msVBIB + msStencil + msSrbLookup;
        const double msUpdate = toMs(m_PerfCounters.rmlUpdateUs);
        const double msRender = toMs(m_PerfCounters.rmlRenderUs);
        const double msFlush = toMs(m_PerfCounters.flushUs);
        const double msIdleGPU = toMs(m_PerfCounters.idleGPUUs);
        const double msTotal = toMs(m_PerfCounters.frameTotalUs);
        const double msBackend = msSum;
        const double msOther = msTotal - msUpdate - msRender - msBackend - msFlush - msIdleGPU
            - toMs(m_PerfCounters.beginFrameUs) - toMs(m_PerfCounters.endFrameUs);

        char buf[1536];
        snprintf(buf, sizeof(buf),
            "[RmlDiligent-Perf] frame=%u "
            "CompileGeom=%u(+%u,%.1fms) Total=%.1fms | "
            "ReleaseGeom=%u CreateBuf=%u RenderGeom=%u "
            "SetPSO=%u/%u CommitSRB=%u/%u DrawIdx=%u MapCB=%u\n"
            "[RmlDiligent-Time] Update=%.2fms Render=%.2fms Backend=%.2fms "
            "Flush=%.2fms IdleGPU=%.2fms "
            "Begin=%.2fms End=%.2fms Other=%.2fms Total=%.2fms\n"
            "[RmlDiligent-Draw] PSO=%.2fms SRB=%.2fms Map=%.2fms Draw=%.2fms "
            "Scissor=%.2fms VBIB=%.2fms Stencil=%.2fms SrbLookup=%.2fms (sum=%.2fms)\n",
            s_DiagFrame,
            m_PerfCounters.compileGeometry, dCompile, msCompileDelta, msCompileTotal,
            m_PerfCounters.releaseGeometry,
            m_PerfCounters.createBuffer,
            m_PerfCounters.renderGeometry,
            m_PerfCounters.setPSOReal, m_PerfCounters.setPSOReq,
            m_PerfCounters.commitSRBReal, m_PerfCounters.commitSRBReq,
            m_PerfCounters.drawIndexed,
            m_PerfCounters.mapCB,
            msUpdate, msRender, msBackend,
            msFlush, msIdleGPU,
            toMs(m_PerfCounters.beginFrameUs),
            toMs(m_PerfCounters.endFrameUs),
            msOther, msTotal,
            msPSO, msSRB, msMap, msDraw,
            msScissor, msVBIB, msStencil, msSrbLookup, msSum);
        OutputDebugStringA(buf);
    }
#endif
}

// =============================================================================
// 帧计时辅助函数（应用层在 Present 前后调用）
// =============================================================================

#ifdef RML_PERF_COUNTERS
void RmlDiligentRenderInterface::MarkPresentStart()
{
    QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&m_PerfCounters.presentStartTick));
}

void RmlDiligentRenderInterface::MarkPresentEnd()
{
    QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&m_PerfCounters.presentEndTick));
}

void RmlDiligentRenderInterface::MarkEndFrameStart()
{
    QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&m_PerfCounters.endFrameStartTick));
}

void RmlDiligentRenderInterface::MarkUpdateStart()
{
    QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&m_PerfCounters.updateStartTick));
}

void RmlDiligentRenderInterface::MarkUpdateEnd()
{
    QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&m_PerfCounters.updateEndTick));
}

void RmlDiligentRenderInterface::MarkRenderStart()
{
    QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&m_PerfCounters.renderStartTick));
}

void RmlDiligentRenderInterface::MarkRenderEnd()
{
    QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&m_PerfCounters.renderEndTick));
}

void RmlDiligentRenderInterface::MarkFlushStart()
{
    QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&m_PerfCounters.flushStartTick));
}

void RmlDiligentRenderInterface::MarkFlushEnd()
{
    QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&m_PerfCounters.flushEndTick));
}

void RmlDiligentRenderInterface::MarkIdleGPUStart()
{
    QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&m_PerfCounters.idleGPUStartTick));
}

void RmlDiligentRenderInterface::MarkIdleGPUEnd()
{
    QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&m_PerfCounters.idleGPUEndTick));
}
#endif

void RmlDiligentRenderInterface::DrawDebugQuad()
{
    const Rml::ColourbPremultiplied red(255, 0, 0, 255);
    Rml::Vertex vertices[4] = {
        {{50.0f, 50.0f}, red, {0.0f, 0.0f}},
        {{450.0f, 50.0f}, red, {1.0f, 0.0f}},
        {{450.0f, 250.0f}, red, {1.0f, 1.0f}},
        {{50.0f, 250.0f}, red, {0.0f, 1.0f}},
    };
    const int indices[6] = {0, 1, 2, 0, 2, 3};

    Rml::CompiledGeometryHandle handle = CompileGeometry(
        Rml::Span<const Rml::Vertex>(vertices, 4),
        Rml::Span<const int>(indices, 6));
    if (handle == 0) {
        std::cerr << "[WARN] DrawDebugQuad: CompileGeometry failed" << std::endl;
        return;
    }

    SetTransform(nullptr);
    RenderGeometry(handle, Rml::Vector2f(0, 0), 0);
    ReleaseGeometry(handle);
}

// =============================================================================
// CompileGeometry
// =============================================================================

Rml::CompiledGeometryHandle RmlDiligentRenderInterface::CompileGeometry(
    Rml::Span<const Rml::Vertex> vertices,
    Rml::Span<const int> indices)
{
#ifdef RML_PERF_COUNTERS
    ++m_PerfCounters.compileGeometry;
    LARGE_INTEGER compT0, compT1;
    QueryPerformanceCounter(&compT0);
#endif

    auto* handle = new GeometryHandle();

    Diligent::BufferDesc vbDesc;
    vbDesc.Name = "RmlDiligent VB";
    vbDesc.Size = vertices.size() * sizeof(Rml::Vertex);
    vbDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
    vbDesc.Usage = Diligent::USAGE_IMMUTABLE;

    Diligent::BufferData vbData(vertices.data(), vbDesc.Size);
    m_Device->CreateBuffer(vbDesc, &vbData, &handle->vbAlloc.buffer);
#ifdef RML_PERF_COUNTERS
    ++m_PerfCounters.createBuffer;
#endif
    if (!handle->vbAlloc.buffer) {
        delete handle;
        return 0;
    }

    Diligent::BufferDesc ibDesc;
    ibDesc.Name = "RmlDiligent IB";
    ibDesc.Size = indices.size() * sizeof(int);
    ibDesc.BindFlags = Diligent::BIND_INDEX_BUFFER;
    ibDesc.Usage = Diligent::USAGE_IMMUTABLE;

    Diligent::BufferData ibData(indices.data(), ibDesc.Size);
    m_Device->CreateBuffer(ibDesc, &ibData, &handle->ibAlloc.buffer);
#ifdef RML_PERF_COUNTERS
    ++m_PerfCounters.createBuffer;
#endif
    if (!handle->ibAlloc.buffer) {
        delete handle;
        return 0;
    }

    handle->vertexCount = static_cast<uint32_t>(vertices.size());
    handle->indexCount = static_cast<uint32_t>(indices.size());

#ifdef RML_PERF_COUNTERS
    QueryPerformanceCounter(&compT1);
    m_PerfCounters.compileGeometryUs += compT1.QuadPart - compT0.QuadPart;
#endif

    return reinterpret_cast<Rml::CompiledGeometryHandle>(handle);
}

// =============================================================================
// RenderGeometry
// =============================================================================

// =============================================================================
// RenderGeometry — 纯色/纹理几何；纹理路径使用 TextureHandle.srbCache 避免每 draw 新建 SRB
// =============================================================================

void RmlDiligentRenderInterface::RenderGeometry(
    Rml::CompiledGeometryHandle geometry,
    Rml::Vector2f translation,
    Rml::TextureHandle texture)
{
    auto* geom = reinterpret_cast<GeometryHandle*>(geometry);
    if (!geom || !geom->vbAlloc.buffer || !geom->ibAlloc.buffer || geom->indexCount == 0) {
        return;
    }

#ifdef RML_PERF_COUNTERS
    ++m_PerfCounters.renderGeometry;
#endif

    EnsureFramebufferBound();

    const bool useTexture = (texture != 0 && m_PSO_Texture);
    // SaveLayerAsTexture / box-shadow callbacks: baked texture already includes bg + shadows.
    // StencilEqual here would clip outer shadows to the border-radius mask (RmlUi draws the
    // quad after SetClippingRegion, but the texture content must not be stencil-clipped).
    const bool callbackTexture =
        useTexture && reinterpret_cast<TextureHandle*>(texture)->fromPool;
    Diligent::IPipelineState* pso = nullptr;
    if (m_ClipMaskEnabled && m_UseStencilEqual && !callbackTexture) {
        pso = useTexture ? m_PSO_Texture_StencilEqual : m_PSO_Color_StencilEqual;
    } else {
        pso = useTexture ? m_PSO_Texture : m_PSO_Color;
    }
    if (!pso) {
        return;
    }

    if (m_ClipMaskEnabled && m_UseStencilEqual && !callbackTexture) {
        { RML_PERF_TIMED(m_PerfCounters.setStencilRefUs, m_Context->SetStencilRef(m_StencilTestValue)); }
    }

    const Rml::Matrix4f& identity = Rml::Matrix4f::Identity();
    const bool isTransformed = memcmp(m_Transform.data(), identity.data(), sizeof(float) * 16) != 0;

    // 诊断：RenderGeometry 中 transform 元素的状态
    static uint32_t s_diagDraw = 0;
    if (isTransformed && s_diagDraw < 20) {
        ++s_diagDraw;
        const float* t = m_Transform.data();
        BoxShadowDiagLog(
            "[RG3D] isT=%d cb=%d scissor=%d clip=%d stencil=%d T[0]=%.4f T[5]=%.4f",
            (int)isTransformed, (int)callbackTexture, (int)m_ScissorEnabled,
            (int)m_ClipMaskEnabled, (int)m_StencilTestValue, t[0], t[5]);
    }
    const bool restoreScissor = callbackTexture && m_ScissorEnabled;
    const bool restoreGeometryScissor = isTransformed && m_ScissorEnabled && !callbackTexture;
    // 3D 变换时，临时把 m_ScissorRegionRml 设为全屏。
    // DrawIndexedGeometry 内部会在绘制后恢复 m_ScissorRegionRml 到 GPU scissor，
    // 如果不改 m_ScissorRegionRml，恢复的 scissor 会裁剪 3D 变换后的顶点。
    Rml::Rectanglei savedScissorRegion;
    if (restoreGeometryScissor) {
        savedScissorRegion = m_ScissorRegionRml;
        const int sw = m_CachedWidth > 0 ? m_CachedWidth : static_cast<int>(m_SwapChain->GetDesc().Width);
        const int sh = m_CachedHeight > 0 ? m_CachedHeight : static_cast<int>(m_SwapChain->GetDesc().Height);
        m_ScissorRegionRml = Rml::Rectanglei::FromCorners(Rml::Vector2i(0, 0), Rml::Vector2i(sw, sh));
        EnableScissorRegion(false);
    }
    if (restoreScissor) {
        EnableScissorRegion(false);
    }

    BindPSO(pso);

    {
#ifdef RML_PERF_COUNTERS
        LARGE_INTEGER mapT0, mapT1;
        QueryPerformanceCounter(&mapT0);
#endif
        Diligent::MapHelper<MainCB> cb(m_Context, m_ConstantBuffer, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
        UploadTransformToCB(cb, m_Transform, translation);
#ifdef RML_PERF_COUNTERS
        QueryPerformanceCounter(&mapT1);
        ++m_PerfCounters.mapCB;
        m_PerfCounters.mapCBUs += mapT1.QuadPart - mapT0.QuadPart;
#endif
    }

    // VB/IB 绑定统一在 DrawIndexedGeometry 中执行，此处不重复绑定
    Diligent::IShaderResourceBinding* srb = nullptr;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> srbTexture;

    if (useTexture) {
        auto* tex = reinterpret_cast<TextureHandle*>(texture);
        // StencilEqual PSO 的 implicit signature 不同，需要独立的 SRB 缓存桶
        const ProgramId texProgId = (pso == m_PSO_Texture_StencilEqual) ? ProgramId::TextureStencilEqual : ProgramId::Texture;
#ifdef RML_PERF_COUNTERS
        { RML_PERF_TIMED(m_PerfCounters.srbLookupUs, srbTexture = GetOrCreateTextureSRBRef(tex, texProgId, pso)); }
#else
        srbTexture = GetOrCreateTextureSRBRef(tex, texProgId, pso);
#endif
        srb = srbTexture;
    } else {
        // SRB 必须匹配 PSO 的 implicit signature
        srb = (pso == m_PSO_Color_StencilEqual) ? m_SRB_Color_StencilEqual : m_SRB_Color;
        if (!srb && !m_CBWarnPrinted) {
            std::cerr << "[WARN] ConstantBuffer not bound (color SRB missing)" << std::endl;
            m_CBWarnPrinted = true;
        }
    }

    if (srb) {
        BindSRB(srb);
    }

    DrawIndexedGeometry(geom);

    if (restoreGeometryScissor) {
        // 恢复 m_ScissorRegionRml 到原始值（RmlUi 设置的元素 2D 边界）
        m_ScissorRegionRml = savedScissorRegion;
        EnableScissorRegion(true);
    }
    if (restoreScissor) {
        EnableScissorRegion(true);
    }

    ++m_DrawCount;
    if (useTexture) {
        ++m_TextureDrawCount;
        if (callbackTexture) {
            ++m_CallbackTextureDrawCount;
        }
    }
}

// =============================================================================
// ReleaseGeometry
// =============================================================================

void RmlDiligentRenderInterface::ReleaseGeometry(Rml::CompiledGeometryHandle geometry)
{
    if (geometry != 0) {
#ifdef RML_PERF_COUNTERS
        ++m_PerfCounters.releaseGeometry;
        LARGE_INTEGER relT0, relT1;
        QueryPerformanceCounter(&relT0);
#endif
        delete reinterpret_cast<GeometryHandle*>(geometry);
#ifdef RML_PERF_COUNTERS
        QueryPerformanceCounter(&relT1);
        m_PerfCounters.releaseGeometryUs += relT1.QuadPart - relT0.QuadPart;
#endif
    }
}

// =============================================================================
// LoadTexture
// =============================================================================

Rml::TextureHandle RmlDiligentRenderInterface::LoadTexture(
    Rml::Vector2i& texture_dimensions,
    const Rml::String& source)
{
    Rml::FileInterface* file_interface = Rml::GetFileInterface();
    Rml::FileHandle file_handle = file_interface->Open(source);
    if (!file_handle) {
        return {};
    }

    file_interface->Seek(file_handle, 0, SEEK_END);
    size_t buffer_size = file_interface->Tell(file_handle);
    file_interface->Seek(file_handle, 0, SEEK_SET);

    if (buffer_size <= sizeof(TGAHeader)) {
        Rml::Log::Message(Rml::Log::LT_ERROR, "Texture file size is smaller than TGAHeader, file is not a valid TGA image.");
        file_interface->Close(file_handle);
        return {};
    }

    std::unique_ptr<Rml::byte[]> buffer(new Rml::byte[buffer_size]);
    file_interface->Read(buffer.get(), buffer_size, file_handle);
    file_interface->Close(file_handle);

    TGAHeader header;
    memcpy(&header, buffer.get(), sizeof(TGAHeader));

    int color_mode = header.bitsPerPixel / 8;
    const int width = header.width[0] + (header.width[1] << 8);
    const int height = header.height[0] + (header.height[1] << 8);
    const size_t image_size = static_cast<size_t>(width) * static_cast<size_t>(height) * 4;

    if (header.dataType != 2) {
        Rml::Log::Message(Rml::Log::LT_ERROR, "Only 24/32bit uncompressed TGAs are supported.");
        return {};
    }

    if (color_mode < 3) {
        Rml::Log::Message(Rml::Log::LT_ERROR, "Only 24 and 32bit textures are supported.");
        return {};
    }

    if (width <= 0 || height <= 0 || width > 8192 || height > 8192) {
        Rml::Log::Message(Rml::Log::LT_ERROR, "Invalid TGA dimensions.");
        return {};
    }

    const Rml::byte* image_src = buffer.get() + sizeof(TGAHeader);
    std::unique_ptr<Rml::byte[]> image_dest_buffer(new Rml::byte[image_size]);
    Rml::byte* image_dest = image_dest_buffer.get();

    for (long y = 0; y < height; y++) {
        long read_index = y * width * color_mode;
        long write_index = ((header.imageDescriptor & 32) != 0) ? read_index : (height - y - 1) * width * 4;
        for (long x = 0; x < width; x++) {
            image_dest[write_index] = image_src[read_index + 2];
            image_dest[write_index + 1] = image_src[read_index + 1];
            image_dest[write_index + 2] = image_src[read_index];
            if (color_mode == 4) {
                const Rml::byte alpha = image_src[read_index + 3];
                for (size_t j = 0; j < 3; j++) {
                    image_dest[write_index + j] = static_cast<Rml::byte>((image_dest[write_index + j] * alpha) / 255);
                }
                image_dest[write_index + 3] = alpha;
            } else {
                image_dest[write_index + 3] = 255;
            }

            write_index += 4;
            read_index += color_mode;
        }
    }

    texture_dimensions.x = width;
    texture_dimensions.y = height;

    return GenerateTexture({image_dest, image_size}, texture_dimensions);
}

// =============================================================================
// GenerateTexture
// =============================================================================

Rml::TextureHandle RmlDiligentRenderInterface::GenerateTexture(
    Rml::Span<const Rml::byte> source,
    Rml::Vector2i source_dimensions)
{
    auto* handle = new TextureHandle();

    Diligent::TextureDesc texDesc;
    texDesc.Name = "RmlDiligent Texture";
    texDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
    texDesc.Width = source_dimensions.x;
    texDesc.Height = source_dimensions.y;
    texDesc.MipLevels = 1;
    texDesc.Format = Diligent::TEX_FORMAT_RGBA8_UNORM;
    texDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;

    Diligent::TextureSubResData subResData;
    subResData.pData = source.data();
    subResData.Stride = source_dimensions.x * 4;

    Diligent::TextureData texData;
    texData.pSubResources = &subResData;
    texData.NumSubresources = 1;

    m_Device->CreateTexture(texDesc, &texData, &handle->texture);

    if (handle->texture) {
        handle->SRV = handle->texture->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);
    }

    return reinterpret_cast<Rml::TextureHandle>(handle);
}

// =============================================================================
// ReleaseTexture
// =============================================================================

void RmlDiligentRenderInterface::ReleaseTexture(Rml::TextureHandle texture)
{
    if (texture == 0) {
        return;
    }
    auto* handle = reinterpret_cast<TextureHandle*>(texture);
    if (!handle->srbCache.empty()) {
        m_TextureSrbEntryCount -= handle->srbCache.size();
        handle->srbCache.clear();
    }
    if (handle->fromPool) {
        handle->pooled.reset();
    }
    delete handle;
}

// =============================================================================
// EnableScissorRegion
// =============================================================================

void RmlDiligentRenderInterface::EnableScissorRegion(bool enable)
{
    m_ScissorEnabled = enable;
    if (!enable && m_Context && m_SwapChain) {
        Diligent::Rect fullScissor{};
        fullScissor.left = 0;
        fullScissor.top = 0;
        fullScissor.right = static_cast<long>(m_SwapChain->GetDesc().Width);
        fullScissor.bottom = static_cast<long>(m_SwapChain->GetDesc().Height);
        m_Context->SetScissorRects(1, &fullScissor, 0, 0);
    } else if (enable && m_ScissorRegionRml.Valid()) {
        SetScissorRml(m_ScissorRegionRml, false);
    }
}

// =============================================================================
// SetScissorRegion
// =============================================================================

void RmlDiligentRenderInterface::SetScissorRegion(Rml::Rectanglei region)
{
    m_ScissorRegionRml = region;
    m_ScissorRect.left = region.Left();
    m_ScissorRect.top = region.Top();
    m_ScissorRect.right = region.Right();
    m_ScissorRect.bottom = region.Bottom();
    if (region.Valid()) {
        SetScissorRml(region, false);
    }
}

// =============================================================================
// SetTransform
// =============================================================================

void RmlDiligentRenderInterface::SetTransform(const Rml::Matrix4f* transform)
{
    // CSS transform（不含投影），shader 中：Projection * (Transform * pos + translate)
    m_Transform = transform ? *transform : Rml::Matrix4f::Identity();
    // 上传投影矩阵到全局 ProjectionBuffer（一次 per-frame，在 SetProjectionMatrix 中设置）
}

void RmlDiligentRenderInterface::EnableClipMask(bool enable)
{
    // Match RmlUi_Renderer_DX12::EnableClipMask — only toggle clip-mask testing; keep m_UseStencilEqual intact.
    m_ClipMaskEnabled = enable;
    if (m_Context) {
        { RML_PERF_TIMED(m_PerfCounters.setStencilRefUs, m_Context->SetStencilRef(enable ? m_StencilTestValue : 0)); }
    }
}

void RmlDiligentRenderInterface::RenderToClipMask(
    Rml::ClipMaskOperation operation,
    Rml::CompiledGeometryHandle geometry,
    Rml::Vector2f translation)
{
    auto* geom = reinterpret_cast<GeometryHandle*>(geometry);
    if (!geom || !m_PSO_Color_StencilSet || !m_PSO_Color_StencilIntersect) {
        return;
    }

    const bool clear_stencil =
        (operation == Rml::ClipMaskOperation::Set || operation == Rml::ClipMaskOperation::SetInverse);
    if (clear_stencil) {
        if (m_SwapchainPassActive) {
            m_Context->EndRenderPass();
            m_SwapchainPassActive = false;
        }
        EnsureFramebufferBound();
        if (auto* pDSV = GetActiveDepthStencilDSV()) {
            m_Context->ClearDepthStencil(pDSV, Diligent::CLEAR_STENCIL_FLAG, 0.0f, 0,
                Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        }
    }

    EnsureFramebufferBound();

    uint8_t stencil_test_value = m_StencilTestValue;
    Diligent::IPipelineState* maskPSO = nullptr;
    uint8_t write_ref = 1;

    switch (operation) {
        case Rml::ClipMaskOperation::Set:
            maskPSO = m_PSO_Color_StencilSet;
            stencil_test_value = 1;
            write_ref = 1;
            break;
        case Rml::ClipMaskOperation::SetInverse:
            // Match GL3/DX12: mask pass always writes ref 1; only the subsequent equal-test ref differs.
            maskPSO = m_PSO_Color_StencilSet;
            stencil_test_value = 0;
            write_ref = 1;
            break;
        case Rml::ClipMaskOperation::Intersect:
            maskPSO = m_PSO_Color_StencilIntersect;
            stencil_test_value = static_cast<uint8_t>(m_StencilTestValue + 1);
            write_ref = 1;
            break;
        default:
            return;
    }

    { RML_PERF_TIMED(m_PerfCounters.setStencilRefUs, m_Context->SetStencilRef(write_ref)); }
    DrawColorGeometry(geom, translation, maskPSO);

    m_StencilTestValue = stencil_test_value;
    m_UseStencilEqual = true;
    m_Context->SetStencilRef(m_StencilTestValue);
    ++m_ClipMaskDrawCount;

    static uint32_t s_clipDiag = 0;
    if (s_clipDiag < 20) {
        ++s_clipDiag;
        BoxShadowDiagLog("[CLIP] op=%d stencilTestVal=%d writeRef=%d",
            (int)operation, (int)stencil_test_value, (int)write_ref);
    }
}

Rml::CompiledShaderHandle RmlDiligentRenderInterface::CompileShader(
    const Rml::String& name,
    const Rml::Dictionary& parameters)
{
    auto applyColorStopList = [](CompiledShader& shader, const Rml::Dictionary& shaderParameters) {
        auto it = shaderParameters.find("color_stop_list");
        if (it == shaderParameters.end() || it->second.GetType() != Rml::Variant::COLORSTOPLIST) {
            return;
        }
        const Rml::ColorStopList& color_stop_list = it->second.GetReference<Rml::ColorStopList>();
        const int num_stops = Rml::Math::Min(static_cast<int>(color_stop_list.size()), kMaxNumStops);

        shader.stopPositions.resize(num_stops);
        shader.stopColors.resize(num_stops);
        for (int i = 0; i < num_stops; ++i) {
            const Rml::ColorStop& stop = color_stop_list[i];
            shader.stopPositions[i] = stop.position.number;
            shader.stopColors[i] = ConvertToColorf(stop.color);
        }
    };

    CompiledShader compiled{};

    if (name == "linear-gradient") {
        compiled.type = CompiledShaderType::Gradient;
        const bool repeating = Rml::Get(parameters, "repeating", false);
        compiled.gradientFunc = repeating ? static_cast<int>(ShaderGradientFunction::RepeatingLinear)
                                          : static_cast<int>(ShaderGradientFunction::Linear);
        compiled.p = Rml::Get(parameters, "p0", Rml::Vector2f(0.f));
        compiled.v = Rml::Get(parameters, "p1", Rml::Vector2f(0.f)) - compiled.p;
        applyColorStopList(compiled, parameters);
    } else if (name == "radial-gradient") {
        compiled.type = CompiledShaderType::Gradient;
        const bool repeating = Rml::Get(parameters, "repeating", false);
        compiled.gradientFunc = repeating ? static_cast<int>(ShaderGradientFunction::RepeatingRadial)
                                          : static_cast<int>(ShaderGradientFunction::Radial);
        compiled.p = Rml::Get(parameters, "center", Rml::Vector2f(0.f));
        compiled.v = Rml::Vector2f(1.f) / Rml::Get(parameters, "radius", Rml::Vector2f(1.f));
        applyColorStopList(compiled, parameters);
    } else if (name == "conic-gradient") {
        compiled.type = CompiledShaderType::Gradient;
        const bool repeating = Rml::Get(parameters, "repeating", false);
        compiled.gradientFunc = repeating ? static_cast<int>(ShaderGradientFunction::RepeatingConic)
                                          : static_cast<int>(ShaderGradientFunction::Conic);
        compiled.p = Rml::Get(parameters, "center", Rml::Vector2f(0.f));
        const float angle = Rml::Get(parameters, "angle", 0.f);
        compiled.v = {Rml::Math::Cos(angle), Rml::Math::Sin(angle)};
        applyColorStopList(compiled, parameters);
    } else if (name == "shader") {
        const Rml::String value = Rml::Get(parameters, "value", Rml::String());
        if (value == "creation") {
            compiled.type = CompiledShaderType::Creation;
            compiled.dimensions = Rml::Get(parameters, "dimensions", Rml::Vector2f(0.f));
        }
    }

    if (compiled.type != CompiledShaderType::Invalid) {
        ++m_CompileShaderCount;
        return reinterpret_cast<Rml::CompiledShaderHandle>(new CompiledShader(std::move(compiled)));
    }

    Rml::Log::Message(Rml::Log::LT_WARNING, "Unsupported shader type '%s'.", name.c_str());
    return {};
}

void RmlDiligentRenderInterface::RenderShader(
    Rml::CompiledShaderHandle shader_handle,
    Rml::CompiledGeometryHandle geometry_handle,
    Rml::Vector2f translation,
    Rml::TextureHandle /*texture*/)
{
    if (!shader_handle || !geometry_handle) {
        return;
    }

    auto* shader = reinterpret_cast<CompiledShader*>(shader_handle);
    auto* geom = reinterpret_cast<GeometryHandle*>(geometry_handle);
    if (!geom || !geom->vbAlloc.buffer || !geom->ibAlloc.buffer || geom->indexCount == 0) {
        return;
    }

#ifdef RML_PERF_COUNTERS
    ++m_PerfCounters.renderShader;
#endif

    EnsureFramebufferBound();

    switch (shader->type) {
        case CompiledShaderType::Gradient: {
            Diligent::IPipelineState* pso =
                (m_ClipMaskEnabled && m_UseStencilEqual) ? m_PSO_Gradient_StencilEqual : m_PSO_Gradient;
            if (!pso || !m_GradientCB) {
                return;
            }

            if (m_ClipMaskEnabled && m_UseStencilEqual) {
                { RML_PERF_TIMED(m_PerfCounters.setStencilRefUs, m_Context->SetStencilRef(m_StencilTestValue)); }
            }
            BindPSO(pso);

            {
                Diligent::MapHelper<GradientCB> cb(m_Context, m_GradientCB, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
                UploadTransformToGradientCB(cb, m_Transform, translation);
                cb->func = shader->gradientFunc;
                cb->numStops = static_cast<int>(shader->stopPositions.size());
                cb->p[0] = shader->p.x;
                cb->p[1] = shader->p.y;
                cb->v[0] = shader->v.x;
                cb->v[1] = shader->v.y;
                memset(cb->stopColors, 0, sizeof(cb->stopColors));
                memset(cb->stopPositions, 0, sizeof(cb->stopPositions));
                for (int i = 0; i < cb->numStops; ++i) {
                    cb->stopColors[i][0] = shader->stopColors[i][0];
                    cb->stopColors[i][1] = shader->stopColors[i][1];
                    cb->stopColors[i][2] = shader->stopColors[i][2];
                    cb->stopColors[i][3] = shader->stopColors[i][3];
                    cb->stopPositions[i >> 2][i & 3] = shader->stopPositions[i];
                }
#ifdef RML_PERF_COUNTERS
                ++m_PerfCounters.mapCB;
#endif
            }

            {
                Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> srb = GetOrCreateCachedSRB(
                    pso, nullptr, m_GradientCB, [&](Diligent::IShaderResourceBinding* binding) {
                        if (auto* cbVar = binding->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, "ConstantBuffer")) {
                            cbVar->Set(m_GradientCB);
                        }
                        if (auto* projVar = binding->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, "ProjectionBuffer")) {
                            projVar->Set(m_ProjectionCB);
                        }
                        if (auto* cbVar = binding->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "SharedConstantBuffer")) {
                            cbVar->Set(m_GradientCB);
                        }
                    });
                BindSRB(srb.RawPtr());
            }
            DrawIndexedGeometry(geom);
            ++m_DrawCount;
            ++m_ShaderDrawCount;
        } break;

        case CompiledShaderType::Creation: {
            Diligent::IPipelineState* pso =
                (m_ClipMaskEnabled && m_UseStencilEqual) ? m_PSO_Creation_StencilEqual : m_PSO_Creation;
            if (!pso || !m_CreationCB) {
                return;
            }

            if (m_ClipMaskEnabled && m_UseStencilEqual) {
                { RML_PERF_TIMED(m_PerfCounters.setStencilRefUs, m_Context->SetStencilRef(m_StencilTestValue)); }
            }
            BindPSO(pso);

            {
                Diligent::MapHelper<CreationCB> cb(m_Context, m_CreationCB, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
                UploadTransformToCreationCB(cb, m_Transform, translation);
                cb->dimensions[0] = shader->dimensions.x;
                cb->dimensions[1] = shader->dimensions.y;
                cb->value = static_cast<float>(Rml::GetSystemInterface()->GetElapsedTime());
                cb->padding[0] = 0.0f;
                cb->padding[1] = 0.0f;
                cb->padding[2] = 0.0f;
#ifdef RML_PERF_COUNTERS
                ++m_PerfCounters.mapCB;
#endif
            }

            {
                Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> srb = GetOrCreateCachedSRB(
                    pso, nullptr, m_CreationCB, [&](Diligent::IShaderResourceBinding* binding) {
                        if (auto* cbVar = binding->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, "ConstantBuffer")) {
                            cbVar->Set(m_CreationCB);
                        }
                        if (auto* projVar = binding->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, "ProjectionBuffer")) {
                            projVar->Set(m_ProjectionCB);
                        }
                        if (auto* cbVar = binding->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "SharedConstantBuffer")) {
                            cbVar->Set(m_CreationCB);
                        }
                    });
                BindSRB(srb.RawPtr());
            }
            DrawIndexedGeometry(geom);
            ++m_DrawCount;
            ++m_ShaderDrawCount;
        } break;

        case CompiledShaderType::Invalid:
        default:
            Rml::Log::Message(Rml::Log::LT_WARNING, "Unhandled render shader type.");
            break;
    }
}

void RmlDiligentRenderInterface::ReleaseShader(Rml::CompiledShaderHandle shader_handle)
{
    if (shader_handle != 0) {
        delete reinterpret_cast<CompiledShader*>(shader_handle);
    }
}

namespace {

void WriteDefaultFullscreenVertices(Diligent::IDeviceContext* context, Diligent::IBuffer* vb)
{
    if (!context || !vb) {
        return;
    }

    const Rml::ColourbPremultiplied white(255, 255, 255, 255);
    const Rml::Vertex vertices[4] = {
        {{-1.0f, -1.0f}, white, {0.0f, 1.0f}},
        {{1.0f, -1.0f}, white, {1.0f, 1.0f}},
        {{1.0f, 1.0f}, white, {1.0f, 0.0f}},
        {{-1.0f, 1.0f}, white, {0.0f, 0.0f}},
    };

    Diligent::MapHelper<Rml::Vertex> mapped(context, vb, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
    if (mapped) {
        memcpy(mapped, vertices, sizeof(vertices));
    }
}

} // namespace

void RmlDiligentRenderInterface::BlitLayerToPostprocessPrimary(Rml::LayerHandle layer_handle)
{
    const auto& source = m_LayerStack.GetLayer(layer_handle);
    auto& destination = m_LayerStack.GetPostprocessPrimary();
    if (!source.texture || !destination.texture) {
        return;
    }

    // Match GL3 note: active scissor must not restrict the layer → postprocess copy.
    const bool previous_scissor_enabled = m_ScissorEnabled;
    m_ScissorEnabled = false;
    if (m_Context && m_SwapChain) {
        const int w = static_cast<int>(m_SwapChain->GetDesc().Width);
        const int h = static_cast<int>(m_SwapChain->GetDesc().Height);
        Diligent::Rect fullScissor{};
        fullScissor.left = 0;
        fullScissor.top = 0;
        fullScissor.right = w;
        fullScissor.bottom = h;
        m_Context->SetScissorRects(1, &fullScissor, static_cast<Diligent::Uint32>(w), static_cast<Diligent::Uint32>(h));
    }

    UnbindRenderTargets();

    if (source.samples > 1) {
        // MSAA：使用 ResolveTextureSubresource 将多采样 RT 解析到单采样 postprocess RT
        Diligent::ResolveTextureSubresourceAttribs resolveAttribs;
        resolveAttribs.SrcTextureTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
        resolveAttribs.DstTextureTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
        m_Context->ResolveTextureSubresource(source.texture, destination.texture, resolveAttribs);
    } else {
        Diligent::CopyTextureAttribs copyAttribs;
        copyAttribs.pSrcTexture = source.texture;
        copyAttribs.pDstTexture = destination.texture;
        copyAttribs.SrcTextureTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
        copyAttribs.DstTextureTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
        m_Context->CopyTexture(copyAttribs);
    }

    m_ScissorEnabled = previous_scissor_enabled;
    if (previous_scissor_enabled && m_ScissorRegionRml.Valid()) {
        SetScissorRml(m_ScissorRegionRml, false);
    }
}

// =============================================================================
// DrawFullscreenPassthrough — 全屏合成；SRB 经 m_SrbCache 按 PSO×SRV 缓存
// =============================================================================

void RmlDiligentRenderInterface::DrawFullscreenPassthrough(
    Diligent::ITextureView* sourceSRV,
    Diligent::ITextureView* destRTV,
    Rml::BlendMode blend_mode,
    bool use_layer_depth_stencil,
    Diligent::IPipelineState* pso_override,
    bool reset_default_uv)
{
    Diligent::IPipelineState* pso = pso_override;
    if (!pso) {
        if (blend_mode == Rml::BlendMode::Replace) {
            // GL3: glDisable(GL_BLEND) for filter/downscale/blit and CompositeLayers(Replace).
            pso = m_PSO_PassthroughReplace;
        } else if (use_layer_depth_stencil) {
            pso = m_PSO_Passthrough;
        } else {
            pso = m_PSO_PassthroughPresent;
        }
    }
    if (!sourceSRV || !destRTV || !pso || !m_FullscreenVB || !m_FullscreenIB) {
        return;
    }

#ifdef RML_PERF_COUNTERS
    ++m_PerfCounters.drawFullscreen;
#endif

    if (reset_default_uv) {
        WriteDefaultFullscreenVertices(m_Context, m_FullscreenVB);
    }

    Diligent::ITextureView* pDSV = use_layer_depth_stencil ? GetActiveDepthStencilDSV() : nullptr;
    m_Context->SetRenderTargets(1, &destRTV, pDSV, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    BindPSO(pso);

    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> srb = GetOrCreateCachedSRB(
        pso, sourceSRV, nullptr, [&](Diligent::IShaderResourceBinding* binding) {
            if (auto* texVar = binding->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_InputTexture")) {
                texVar->Set(sourceSRV);
            }
            if (auto* sampVar = binding->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_SamplerLinear")) {
                sampVar->Set(m_Sampler);
            }
        });

    Diligent::Uint64 offset = 0;
    Diligent::IBuffer* pVB = m_FullscreenVB;
    m_Context->SetVertexBuffers(0, 1, &pVB, &offset, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
    m_Context->SetIndexBuffer(m_FullscreenIB, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    BindSRB(srb.RawPtr(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    // Scissor/viewport are set by the caller (filter bounds, blur downscale, layer composite, etc.).
    Diligent::DrawIndexedAttribs drawAttrs(6, Diligent::VT_UINT32, Diligent::DRAW_FLAG_NONE);
    m_Context->DrawIndexed(drawAttrs);
#ifdef RML_PERF_COUNTERS
    ++m_PerfCounters.drawIndexed;
#endif
    UnbindRenderTargets();
    ++m_DrawCount;
}

void RmlDiligentRenderInterface::DrawFullscreenPassthroughUV(
    Diligent::ITextureView* sourceSRV,
    Diligent::ITextureView* destRTV,
    Rml::Vector2f uv_offset,
    Rml::Vector2f uv_scaling,
    Diligent::IPipelineState* pso_override)
{
    if (!sourceSRV || !destRTV || !m_FullscreenVB || !m_FullscreenIB) {
        return;
    }

    const Rml::ColourbPremultiplied white(255, 255, 255, 255);
    Rml::Vertex vertices[4] = {
        {{-1.0f, -1.0f}, white, {0.0f * uv_scaling.x + uv_offset.x, 1.0f * uv_scaling.y + uv_offset.y}},
        {{1.0f, -1.0f}, white, {1.0f * uv_scaling.x + uv_offset.x, 1.0f * uv_scaling.y + uv_offset.y}},
        {{1.0f, 1.0f}, white, {1.0f * uv_scaling.x + uv_offset.x, 0.0f * uv_scaling.y + uv_offset.y}},
        {{-1.0f, 1.0f}, white, {0.0f * uv_scaling.x + uv_offset.x, 0.0f * uv_scaling.y + uv_offset.y}},
    };

    {
        Diligent::MapHelper<Rml::Vertex> mapped(m_Context, m_FullscreenVB, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
        if (mapped) {
            memcpy(mapped, vertices, sizeof(vertices));
        }
    }

    DrawFullscreenPassthrough(sourceSRV, destRTV, Rml::BlendMode::Replace, false,
        pso_override ? pso_override : m_PSO_PassthroughReplace, false);
}

Rml::LayerHandle RmlDiligentRenderInterface::PushLayer()
{
    if (m_SwapchainPassActive) {
        m_Context->EndRenderPass();
        m_SwapchainPassActive = false;
    }

    const Rml::LayerHandle handle = m_LayerStack.PushLayer();
    const auto& layer = m_LayerStack.GetLayer(handle);
    Diligent::ITextureView* pRTV = layer.RTV.RawPtr();
    if (!pRTV) {
        return {};
    }

    BindLayer(handle);

    Diligent::OptimizedClearValue clearVal{};
    clearVal.Color[0] = 0.0f;
    clearVal.Color[1] = 0.0f;
    clearVal.Color[2] = 0.0f;
    clearVal.Color[3] = 0.0f;
    m_Context->ClearRenderTarget(pRTV, &clearVal, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    // Match GL3/DX12 PushLayer: clear color only; shared stencil persists across the layer stack.

    ++m_PushLayerCount;
    return handle;
}

void RmlDiligentRenderInterface::CompositeLayers(
    Rml::LayerHandle source,
    Rml::LayerHandle destination,
    Rml::BlendMode blend_mode,
    Rml::Span<const Rml::CompiledFilterHandle> filters)
{
    BlitLayerToPostprocessPrimary(source);

    // 清空 secondary RT，防止上一个 CompositeLayers 的 filter 链残留内容。
    // GL3 最终合成用元素 scissor（残留不可见），Diligent 用全屏 scissor（残留会被写出）。
    // 每个 filter 写 secondary 时只在 scissor 区域内覆盖，scissor 外必须是透明（0）。
    {
        auto& secondary = m_LayerStack.GetPostprocessSecondary();
        if (auto* pRTV = secondary.RTV.RawPtr()) {
            Diligent::OptimizedClearValue zero{};
            zero.Color[0] = zero.Color[1] = zero.Color[2] = zero.Color[3] = 0.0f;
            m_Context->SetRenderTargets(1, &pRTV, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
            m_Context->ClearRenderTarget(pRTV, &zero, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        }
    }

    RenderFilters(filters);
    EnsureFramebufferBound();

    const auto& destLayer = m_LayerStack.GetLayer(destination);
    auto& postPrimary = m_LayerStack.GetPostprocessPrimary();
    if (!postPrimary.SRV || !destLayer.RTV) {
        return;
    }

    if (m_CachedWidth > 0 && m_CachedHeight > 0) {
        SetFramebufferViewport(m_CachedWidth, m_CachedHeight);
    } else {
        SetSwapchainViewport();
    }

    // 与 GL3 一致：使用 RmlUi 设置的当前 scissor（由 ApplyClippingRegion 管理），不覆盖全屏 scissor。
    // backdrop-filter 需要 border-box scissor 裁剪 blur 溢出，全屏 scissor 会导致 blur 溢出形成矩形框。
    if (m_ScissorRegionRml.Valid()) {
        SetScissorRml(m_ScissorRegionRml, false);
    }

    // Stencil 裁剪优先：backdrop-filter 圆角裁剪、box-shadow 残影防护都需要 stencil test。
    // 必须在 MSAA 之前检查，否则 MSAA PSO 会覆盖 stencil 选择。
    Diligent::IPipelineState* composite_pso = nullptr;
    if (m_ClipMaskEnabled && m_UseStencilEqual) {
        // MSAA + StencilEqual 或 非 MSAA + StencilEqual
        composite_pso = (m_MsaaSamples > 1) ? m_PSO_Composite_StencilEqual : m_PSO_Passthrough_StencilEqual;
        { RML_PERF_TIMED(m_PerfCounters.setStencilRefUs, m_Context->SetStencilRef(m_StencilTestValue)); }
    } else if (m_MsaaSamples > 1) {
        composite_pso = (blend_mode == Rml::BlendMode::Replace) ? m_PSO_CompositeReplace : m_PSO_Composite;
    } else if (blend_mode == Rml::BlendMode::Replace) {
        composite_pso = m_PSO_PassthroughReplace;
    }
    DrawFullscreenPassthrough(postPrimary.SRV.RawPtr(), destLayer.RTV.RawPtr(), blend_mode, true, composite_pso);
    UnbindRenderTargets();
    if (m_ScissorRegionRml.Valid()) {
        SetScissorRml(m_ScissorRegionRml, false);
    }
    if (destination != m_LayerStack.GetTopLayerHandle()) {
        BindTopLayer();
    }
    ++m_CompositeCount;
}

void RmlDiligentRenderInterface::PopLayer()
{
    // Match RmlUi_Renderer_DX12::PopLayer — only pop stack and rebind top layer; never clear stencil.
    m_LayerStack.PopLayer();
    if (m_LayerStack.GetLayerCount() > 0) {
        BindTopLayer();
    }
}

void RmlDiligentRenderInterface::UploadBlurCB(
    float sigma,
    const Rml::Vector2f& texel_offset,
    const Rml::Rectanglei& scissor,
    const Rml::Vector2i& framebuffer_size)
{
    if (!m_BlurCB) {
        return;
    }

    Rml::Rectanglei region = scissor;
    if (!region.Valid()) {
        region.p0 = Rml::Vector2i(0, 0);
        region.p1 = framebuffer_size;
    }
    const Rml::Vector2f min_coord = (Rml::Vector2f(region.p0) + Rml::Vector2f(0.5f)) / Rml::Vector2f(framebuffer_size);
    const Rml::Vector2f max_coord = (Rml::Vector2f(region.p1) - Rml::Vector2f(0.5f)) / Rml::Vector2f(framebuffer_size);

    Diligent::MapHelper<BlurCB> cb(m_Context, m_BlurCB, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
    memset(cb->transform, 0, sizeof(cb->transform));
    cb->transform[0] = cb->transform[5] = cb->transform[10] = cb->transform[15] = 1.0f;
    cb->translate[0] = 0.0f;
    cb->translate[1] = 0.0f;
    cb->texelOffset[0] = texel_offset.x;
    cb->texelOffset[1] = texel_offset.y;
    SetBlurWeights(cb->weights, 4, sigma);
    cb->texCoordMin[0] = min_coord.x;
    cb->texCoordMin[1] = min_coord.y;
    cb->texCoordMax[0] = max_coord.x;
    cb->texCoordMax[1] = max_coord.y;
}

void RmlDiligentRenderInterface::UploadDropShadowCB(
    const Rml::Colourf& color,
    const Rml::Rectanglei& scissor,
    const Rml::Vector2i& framebuffer_size)
{
    if (!m_DropShadowCB) {
        return;
    }

    Rml::Rectanglei region = scissor;
    if (!region.Valid()) {
        region.p0 = Rml::Vector2i(0, 0);
        region.p1 = framebuffer_size;
    }
    const Rml::Vector2f min_coord = (Rml::Vector2f(region.p0) + Rml::Vector2f(0.5f)) / Rml::Vector2f(framebuffer_size);
    const Rml::Vector2f max_coord = (Rml::Vector2f(region.p1) - Rml::Vector2f(0.5f)) / Rml::Vector2f(framebuffer_size);

    Diligent::MapHelper<DropShadowCBData> cb(m_Context, m_DropShadowCB, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
    cb->texCoordMin[0] = min_coord.x;
    cb->texCoordMin[1] = min_coord.y;
    cb->texCoordMax[0] = max_coord.x;
    cb->texCoordMax[1] = max_coord.y;
    cb->color[0] = color[0];
    cb->color[1] = color[1];
    cb->color[2] = color[2];
    cb->color[3] = color[3];
}

void RmlDiligentRenderInterface::UploadColorMatrixCB(const Rml::Matrix4f& color_matrix)
{
    if (!m_ColorMatrixCB) {
        return;
    }

    Diligent::MapHelper<ColorMatrixCBData> cb(m_Context, m_ColorMatrixCB, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
    // Match RmlUi_Renderer_DX12/GL3: only transpose RowMajorMatrix4f for HLSL row_major mul().
    constexpr bool is_need_transpose = std::is_same_v<decltype(color_matrix), Rml::RowMajorMatrix4f>;
    const float* p_data = is_need_transpose ? color_matrix.Transpose().data() : color_matrix.data();
    memcpy(cb->colorMatrix, p_data, sizeof(float) * 16);
}

void RmlDiligentRenderInterface::SetSwapchainViewport()
{
    if (!m_SwapChain || !m_Context) {
        return;
    }
    SetFramebufferViewport(static_cast<int>(m_SwapChain->GetDesc().Width), static_cast<int>(m_SwapChain->GetDesc().Height));
}

void RmlDiligentRenderInterface::SetFramebufferViewport(int width, int height)
{
    if (!m_Context || width <= 0 || height <= 0) {
        return;
    }
    Diligent::Viewport vp{};
    vp.Width = static_cast<float>(width);
    vp.Height = static_cast<float>(height);
    vp.MinDepth = 0.f;
    vp.MaxDepth = 1.f;
    m_Context->SetViewports(1, &vp, static_cast<Diligent::Uint32>(width), static_cast<Diligent::Uint32>(height));
}

void RmlDiligentRenderInterface::SetScissorRml(const Rml::Rectanglei& region, bool vertically_flip)
{
    if (!m_Context || !region.Valid()) {
        return;
    }

    const int viewport_width =
        m_CachedWidth > 0 ? m_CachedWidth : (m_SwapChain ? static_cast<int>(m_SwapChain->GetDesc().Width) : 0);
    const int viewport_height =
        m_CachedHeight > 0 ? m_CachedHeight : (m_SwapChain ? static_cast<int>(m_SwapChain->GetDesc().Height) : 0);
    if (viewport_width <= 0 || viewport_height <= 0) {
        return;
    }

    Rml::Rectanglei gpu_region = region;
    if (vertically_flip) {
        gpu_region = VerticallyFlipped(gpu_region, viewport_height);
    }

    const int x_min = Rml::Math::Clamp(gpu_region.Left(), 0, viewport_width);
    const int y_min = Rml::Math::Clamp(gpu_region.Top(), 0, viewport_height);
    const int x_max = Rml::Math::Clamp(gpu_region.Right(), 0, viewport_width);
    const int y_max = Rml::Math::Clamp(gpu_region.Bottom(), 0, viewport_height);

    Diligent::Rect scissor{};
    scissor.left = x_min;
    scissor.top = y_min;
    scissor.right = x_max;
    scissor.bottom = y_max;
    m_Context->SetScissorRects(1, &scissor, static_cast<Diligent::Uint32>(viewport_width),
        static_cast<Diligent::Uint32>(viewport_height));
}

// Subregion blit for blur upscale — RmlUi_Renderer_DX12::BlitFramebuffer (stretched path).
void RmlDiligentRenderInterface::BlitFramebuffer(
    Diligent::ITextureView* sourceSRV,
    Diligent::ITextureView* destRTV,
    int fb_width,
    int fb_height,
    int srcX0,
    int srcY0,
    int srcX1,
    int srcY1,
    int dstX0,
    int dstY0,
    int dstX1,
    int dstY1)
{
    if (!sourceSRV || !destRTV || !m_FullscreenVB || !m_FullscreenIB || !m_PSO_PassthroughReplace) {
        return;
    }

    const int src_width = srcX1 - srcX0;
    const int src_height = srcY1 - srcY0;
    const int dest_width = dstX1 - dstX0;
    const int dest_height = dstY1 - dstY0;
    // DX12 allows flipped/stretched blits (negative width/height). Zero-area only when an edge collapses.
    if (srcX0 == srcX1 || srcY0 == srcY1 || dstX0 == dstX1 || dstY0 == dstY1) {
        return;
    }

    const float uv_x_min = static_cast<float>(srcX0) / static_cast<float>(fb_width);
    const float uv_y_max = static_cast<float>(srcY0) / static_cast<float>(fb_height);
    const float uv_x_max = static_cast<float>(srcX1) / static_cast<float>(fb_width);
    const float uv_y_min = static_cast<float>(srcY1) / static_cast<float>(fb_height);

    const float pos_x_min = (static_cast<float>(dstX0) / static_cast<float>(fb_width)) * 2.0f - 1.0f;
    const float pos_x_max = ((static_cast<float>(dstX0) + static_cast<float>(dest_width)) / static_cast<float>(fb_width)) * 2.0f - 1.0f;
    // Diligent/D3D screen space here follows the same top-left convention as the rest of the RmlUi
    // renderer. For a sub-rectangle blit, convert the top-left pixel bounds directly to NDC.
    const float pos_y_max = 1.0f - (2.0f * static_cast<float>(dstY0) / static_cast<float>(fb_height));
    const float pos_y_min = 1.0f - (2.0f * static_cast<float>(dstY0 + dest_height) / static_cast<float>(fb_height));

    const Rml::ColourbPremultiplied white(255, 255, 255, 255);
    const Rml::Vertex vertices[4] = {
        {{pos_x_min, pos_y_min}, white, {uv_x_min, uv_y_min}},
        {{pos_x_max, pos_y_min}, white, {uv_x_max, uv_y_min}},
        {{pos_x_max, pos_y_max}, white, {uv_x_max, uv_y_max}},
        {{pos_x_min, pos_y_max}, white, {uv_x_min, uv_y_max}},
    };

    Diligent::MapHelper<Rml::Vertex> mappedVerts(m_Context, m_FullscreenVB, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
    if (mappedVerts) {
        memcpy(mappedVerts, vertices, sizeof(vertices));
    }

    Diligent::Viewport vp{};
    vp.TopLeftX = 0.f;
    vp.TopLeftY = 0.f;
    vp.Width = static_cast<float>(fb_width);
    vp.Height = static_cast<float>(fb_height);
    vp.MinDepth = 0.f;
    vp.MaxDepth = 1.f;
    m_Context->SetViewports(1, &vp, static_cast<Diligent::Uint32>(fb_width), static_cast<Diligent::Uint32>(fb_height));

    //Diligent::Rect scissor{};
    //scissor.left = 0;
    //scissor.top = 0;
    //scissor.right = fb_width;
    //scissor.bottom = fb_height;
    //m_Context->SetScissorRects(1, &scissor, static_cast<Diligent::Uint32>(fb_width), static_cast<Diligent::Uint32>(fb_height));

    m_Context->SetRenderTargets(1, &destRTV, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    BindPSO(m_PSO_PassthroughReplace);

    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> srb = GetOrCreateCachedSRB(
        m_PSO_PassthroughReplace, sourceSRV, nullptr, [&](Diligent::IShaderResourceBinding* binding) {
            if (auto* texVar = binding->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_InputTexture")) {
                texVar->Set(sourceSRV);
            }
            if (auto* sampVar = binding->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_SamplerLinear")) {
                sampVar->Set(m_Sampler);
            }
        });

    Diligent::Uint64 offset = 0;
    Diligent::IBuffer* pVB = m_FullscreenVB;
    m_Context->SetVertexBuffers(0, 1, &pVB, &offset, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
    m_Context->SetIndexBuffer(m_FullscreenIB, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    BindSRB(srb.RawPtr(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    Diligent::DrawIndexedAttribs drawAttrs(6, Diligent::VT_UINT32, Diligent::DRAW_FLAG_NONE);
    m_Context->DrawIndexed(drawAttrs);
#ifdef RML_PERF_COUNTERS
    ++m_PerfCounters.drawIndexed;
#endif
    ++m_DrawCount;
}

void RmlDiligentRenderInterface::DrawBlurPass(
    Diligent::ITextureView* sourceSRV,
    Diligent::ITextureView* destRTV,
    const Rml::Rectanglei& scissor_rect,
    int fb_width,
    int fb_height)
{
    if (!sourceSRV || !destRTV || !m_PSO_Blur || !m_FullscreenVB || !m_FullscreenIB) {
        return;
    }

    SetFramebufferViewport(fb_width, fb_height);
    SetScissorRml(scissor_rect, false);
    BindPSO(m_PSO_Blur);
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> srb = GetOrCreateCachedSRB(
        m_PSO_Blur, sourceSRV, m_BlurCB, [&](Diligent::IShaderResourceBinding* binding) {
            if (auto* texVar = binding->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_InputTexture")) {
                texVar->Set(sourceSRV);
            }
            if (auto* sampVar = binding->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_SamplerLinear")) {
                sampVar->Set(m_Sampler);
            }
            if (auto* cbVar = binding->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, "SharedConstantBuffer")) {
                cbVar->Set(m_BlurCB);
            }
            if (auto* cbVar = binding->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "SharedConstantBuffer")) {
                cbVar->Set(m_BlurCB);
            }
        });

    Diligent::ITextureView* pRTV = destRTV;
    m_Context->SetRenderTargets(1, &pRTV, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    Diligent::Uint64 offset = 0;
    Diligent::IBuffer* pVB = m_FullscreenVB;
    m_Context->SetVertexBuffers(0, 1, &pVB, &offset, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
    m_Context->SetIndexBuffer(m_FullscreenIB, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    BindSRB(srb.RawPtr(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    Diligent::DrawIndexedAttribs drawAttrs(6, Diligent::VT_UINT32, Diligent::DRAW_FLAG_NONE);
    m_Context->DrawIndexed(drawAttrs);
#ifdef RML_PERF_COUNTERS
    ++m_PerfCounters.drawIndexed;
#endif
    ++m_DrawCount;
}

// RenderBlur — Diligent/D3D top-left coordinates (GL3 reference; not GL window_flipped space).
void RmlDiligentRenderInterface::RenderBlur(
    float sigma,
    PooledRenderTarget& source_destination,
    PooledRenderTarget& temp,
    const Rml::Rectanglei& blur_region)
{
    if (!source_destination.texture || !temp.texture || !m_PSO_Blur || !blur_region.Valid()) {
        return;
    }

    int pass_level = 0;
    float blur_sigma = sigma;
    SigmaToParameters(sigma, pass_level, blur_sigma);
    if (blur_sigma == 0.f) {
        return;
    }

    const Rml::Rectanglei original_scissor = m_ScissorRegionRml;
    const int fb_w = source_destination.width;
    const int fb_h = source_destination.height;
    const Rml::Rectanglei output_region = blur_region;

    // === 与 GL3 完全一致的 blur 算法 ===
    // GL3: pass_level 次 downscale（halving）→ 在小分辨率上做 2-pass blur → 结果留在小 RT 中
    // Diligent: 用小 RT 实现同样的流程
    int small_w = fb_w, small_h = fb_h;
    for (int i = 0; i < pass_level; ++i) {
        small_w = Rml::Math::Max(1, (small_w + 1) / 2);
        small_h = Rml::Math::Max(1, (small_h + 1) / 2);
    }

    auto rtSmallDst = RenderTargetPool::AcquireColor(m_RTPool, small_w, small_h);
    auto rtSmallTmp = RenderTargetPool::AcquireColor(m_RTPool, small_w, small_h);
    if (!rtSmallDst || !rtSmallDst->texture || !rtSmallTmp || !rtSmallTmp->texture) {
        return;
    }

    UnbindRenderTargets();

    // 1. Downscale: 全分辨率 → 小分辨率（双线性插值）
    SetFramebufferViewport(small_w, small_h);
    const Rml::Rectanglei small_region = Rml::Rectanglei::FromCorners({0, 0}, {small_w, small_h});
    SetScissorRml(small_region, false);
    DrawFullscreenPassthrough(source_destination.SRV.RawPtr(), rtSmallDst->RTV.RawPtr(),
        Rml::BlendMode::Replace, false, m_PSO_PassthroughReplace);

    // 2. Two-pass Gaussian blur at reduced resolution（与 GL3 完全一致的 sigma/texel）
    const int blur_pad = Rml::Math::Max(1, static_cast<int>(std::ceil(blur_sigma * 3.f)) + 2);
    Rml::Rectanglei blur_bounds = small_region.Extend(blur_pad);
    blur_bounds.p0.x = Rml::Math::Clamp(blur_bounds.p0.x, 0, small_w);
    blur_bounds.p0.y = Rml::Math::Clamp(blur_bounds.p0.y, 0, small_h);
    blur_bounds.p1.x = Rml::Math::Clamp(blur_bounds.p1.x, 0, small_w);
    blur_bounds.p1.y = Rml::Math::Clamp(blur_bounds.p1.y, 0, small_h);

    SetFramebufferViewport(small_w, small_h);
    SetScissorRml(small_region, false);

    // Vertical blur: dst → tmp
    UploadBlurCB(blur_sigma, {0.f, 1.f / static_cast<float>(small_h)}, blur_bounds, {small_w, small_h});
    DrawBlurPass(rtSmallDst->SRV.RawPtr(), rtSmallTmp->RTV.RawPtr(), blur_bounds, small_w, small_h);

    // Clear dst for horizontal pass
    {
        Diligent::OptimizedClearValue clr{};
        clr.Color[0] = clr.Color[1] = clr.Color[2] = clr.Color[3] = 0.f;
        Diligent::ITextureView* pRTV = rtSmallDst->RTV.RawPtr();
        m_Context->SetRenderTargets(1, &pRTV, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        SetScissorRml(blur_bounds, false);
        m_Context->ClearRenderTarget(pRTV, &clr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        SetScissorRml(small_region, false);
    }

    // Horizontal blur: tmp → dst
    UploadBlurCB(blur_sigma, {1.f / static_cast<float>(small_w), 0.f}, blur_bounds, {small_w, small_h});
    DrawBlurPass(rtSmallTmp->SRV.RawPtr(), rtSmallDst->RTV.RawPtr(), blur_bounds, small_w, small_h);

    // 3. Upscale: 小分辨率 blurred result → 全分辨率 source_destination
    SetFramebufferViewport(fb_w, fb_h);
    SetScissorRml(output_region, false);
    DrawFullscreenPassthrough(rtSmallDst->SRV.RawPtr(), source_destination.RTV.RawPtr(),
        Rml::BlendMode::Replace, false, m_PSO_PassthroughReplace);

    // Cleanup
    SetFramebufferViewport(m_CachedWidth > 0 ? m_CachedWidth : fb_w, m_CachedHeight > 0 ? m_CachedHeight : fb_h);
    m_ScissorRegionRml = original_scissor;
    if (original_scissor.Valid()) {
        SetScissorRml(original_scissor, false);
    }

    ++m_FilterRenderCount;
}

// =============================================================================
// RenderFilters — opacity/blur/drop-shadow/color-matrix/mask；各 pass SRB 走 PSO 级缓存
// =============================================================================

void RmlDiligentRenderInterface::RenderFilters(Rml::Span<const Rml::CompiledFilterHandle> filter_handles)
{
    const int w = static_cast<int>(m_SwapChain->GetDesc().Width);
    const int h = static_cast<int>(m_SwapChain->GetDesc().Height);
    const Rml::Rectanglei scissor = [&]() {
        if (m_ScissorRegionRml.Valid()) {
            return m_ScissorRegionRml;
        }
        Rml::Rectanglei full{};
        full.p0 = Rml::Vector2i(0, 0);
        full.p1 = Rml::Vector2i(w, h);
        return full;
    }();

    for (const Rml::CompiledFilterHandle filter_handle : filter_handles) {
        if (!filter_handle) {
            continue;
        }
        const auto& filter = *reinterpret_cast<const CompiledFilter*>(filter_handle);

        switch (filter.type) {
        case FilterType::Passthrough: {
            auto& source = m_LayerStack.GetPostprocessPrimary();
            auto& destination = m_LayerStack.GetPostprocessSecondary();
            if (!source.SRV || !destination.RTV) {
                break;
            }

            SetSwapchainViewport();
            SetScissorRml(scissor);
            const float blend_factors[4] = {filter.blend_factor, filter.blend_factor, filter.blend_factor, filter.blend_factor};
            m_Context->SetBlendFactors(blend_factors);
            DrawFullscreenPassthrough(source.SRV.RawPtr(), destination.RTV.RawPtr(), Rml::BlendMode::Replace, false,
                m_PSO_PassthroughOpacity);
            m_LayerStack.SwapPostprocessPrimarySecondary();
            ++m_FilterRenderCount;
        } break;

        case FilterType::Blur: {
            auto& primary = m_LayerStack.GetPostprocessPrimary();
            auto& secondary = m_LayerStack.GetPostprocessSecondary();
            RenderBlur(filter.sigma, primary, secondary, scissor);
        } break;

        case FilterType::DropShadow: {
            auto& primary = m_LayerStack.GetPostprocessPrimary();
            auto& secondary = m_LayerStack.GetPostprocessSecondary();
            if (!primary.SRV || !secondary.RTV) {
                break;
            }

            UploadDropShadowCB(ConvertToColorf(filter.color), scissor, {primary.width, primary.height});
            // GL3 的 tex coord 空间经过 VerticallyFlipped：UV.y=0 → 屏幕顶部，UV.y↑ → 屏幕下移。
            // Diligent 的 tex coord 空间不翻转：UV.y=0 → 屏幕底部，UV.y↑ → 屏幕上移。
            // 因此 Y 分量需要取反（-H）才能匹配 GL3 的偏移方向。
            const Rml::Vector2f uv_offset = filter.offset / Rml::Vector2f(-static_cast<float>(w), -static_cast<float>(h));
            SetSwapchainViewport();
            SetScissorRml(scissor);
            {
                // 清空 secondary RT，防止残留上一个元素 filter 链的内容。
                // GL3 最终合成用元素 scissor（残留不可见），Diligent 用全屏 scissor（残留会被写出）。
                Diligent::OptimizedClearValue zeroClear{};
                zeroClear.Color[0] = zeroClear.Color[1] = zeroClear.Color[2] = zeroClear.Color[3] = 0.0f;
                Diligent::ITextureView* pClearRTV = secondary.RTV.RawPtr();
                m_Context->SetRenderTargets(1, &pClearRTV, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
                m_Context->ClearRenderTarget(pClearRTV, &zeroClear, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

                BindPSO(m_PSO_DropShadow);
                Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> srb = GetOrCreateCachedSRB(
                    m_PSO_DropShadow, primary.SRV.RawPtr(), m_DropShadowCB, [&](Diligent::IShaderResourceBinding* binding) {
                        if (auto* texVar = binding->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_InputTexture")) {
                            texVar->Set(primary.SRV);
                        }
                        if (auto* sampVar = binding->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_SamplerLinear")) {
                            sampVar->Set(m_Sampler);
                        }
                        if (auto* cbVar = binding->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "DropShadowBuffer")) {
                            cbVar->Set(m_DropShadowCB);
                        }
                    });
                const Rml::ColourbPremultiplied white(255, 255, 255, 255);
                Rml::Vertex vertices[4] = {
                    {{-1.0f, -1.0f}, white, {0.0f + uv_offset.x, 1.0f + uv_offset.y}},
                    {{1.0f, -1.0f}, white, {1.0f + uv_offset.x, 1.0f + uv_offset.y}},
                    {{1.0f, 1.0f}, white, {1.0f + uv_offset.x, 0.0f + uv_offset.y}},
                    {{-1.0f, 1.0f}, white, {0.0f + uv_offset.x, 0.0f + uv_offset.y}},
                };
                Diligent::MapHelper<Rml::Vertex> mapped(m_Context, m_FullscreenVB, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
                if (mapped) {
                    memcpy(mapped, vertices, sizeof(vertices));
                }
                Diligent::ITextureView* pRTV = secondary.RTV.RawPtr();
                m_Context->SetRenderTargets(1, &pRTV, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
                Diligent::Uint64 offset = 0;
                Diligent::IBuffer* pVB = m_FullscreenVB;
                m_Context->SetVertexBuffers(0, 1, &pVB, &offset, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
                m_Context->SetIndexBuffer(m_FullscreenIB, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
                BindSRB(srb.RawPtr(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
                Diligent::DrawIndexedAttribs drawAttrs(6, Diligent::VT_UINT32, Diligent::DRAW_FLAG_NONE);
                m_Context->DrawIndexed(drawAttrs);
#ifdef RML_PERF_COUNTERS
                ++m_PerfCounters.drawIndexed;
#endif
                ++m_DrawCount;
            }
            WriteDefaultFullscreenVertices(m_Context, m_FullscreenVB);

            if (filter.sigma >= 0.5f) {
                auto& tertiary = m_LayerStack.GetPostprocessTertiary();
                RenderBlur(filter.sigma, secondary, tertiary, scissor);
            }

            // 与 GL3 一致：blur 之后，将原始内容（primary）用 alpha blend 叠加到阴影（secondary）上。
            // 这样最终 secondary = 阴影 + 原始内容（文字、背景、图片），swap 后 primary 持有完整结果。
            // 如果缺少这步，只有阴影可见，原始内容丢失。
            DrawFullscreenPassthrough(primary.SRV.RawPtr(), secondary.RTV.RawPtr(), Rml::BlendMode::Blend, false);

            m_LayerStack.SwapPostprocessPrimarySecondary();
            ++m_FilterRenderCount;
        } break;

        case FilterType::ColorMatrix: {
            auto& source = m_LayerStack.GetPostprocessPrimary();
            auto& destination = m_LayerStack.GetPostprocessSecondary();
            if (!source.SRV || !destination.RTV) {
                break;
            }

            UploadColorMatrixCB(filter.color_matrix);
            SetSwapchainViewport();
            SetScissorRml(scissor);
            Diligent::OptimizedClearValue clearVal{};
            clearVal.Color[0] = clearVal.Color[1] = clearVal.Color[2] = clearVal.Color[3] = 0.0f;
            Diligent::ITextureView* pRTV = destination.RTV.RawPtr();
            m_Context->SetRenderTargets(1, &pRTV, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
            m_Context->ClearRenderTarget(pRTV, &clearVal, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
            BindPSO(m_PSO_ColorMatrix);
            Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> srb = GetOrCreateCachedSRB(
                m_PSO_ColorMatrix, source.SRV.RawPtr(), m_ColorMatrixCB, [&](Diligent::IShaderResourceBinding* binding) {
                    if (auto* texVar = binding->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_InputTexture")) {
                        texVar->Set(source.SRV);
                    }
                    if (auto* sampVar = binding->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_SamplerLinear")) {
                        sampVar->Set(m_Sampler);
                    }
                    if (auto* cbVar = binding->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "ConstantBuffer")) {
                        cbVar->Set(m_ColorMatrixCB);
                    }
                });

            Diligent::ITextureView* pRTV1 = destination.RTV.RawPtr();
            m_Context->SetRenderTargets(1, &pRTV1, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
            Diligent::Uint64 offset = 0;
            Diligent::IBuffer* pVB = m_FullscreenVB;
            m_Context->SetVertexBuffers(0, 1, &pVB, &offset, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
            m_Context->SetIndexBuffer(m_FullscreenIB, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
            BindSRB(srb.RawPtr(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
            Diligent::DrawIndexedAttribs drawAttrs(6, Diligent::VT_UINT32, Diligent::DRAW_FLAG_NONE);
            m_Context->DrawIndexed(drawAttrs);
#ifdef RML_PERF_COUNTERS
            ++m_PerfCounters.drawIndexed;
#endif
            ++m_DrawCount;
            m_LayerStack.SwapPostprocessPrimarySecondary();
            ++m_FilterRenderCount;
        } break;

        case FilterType::MaskImage: {
            auto& source = m_LayerStack.GetPostprocessPrimary();
            auto& blend_mask = m_LayerStack.GetBlendMask();
            auto& destination = m_LayerStack.GetPostprocessSecondary();
            if (!source.SRV || !blend_mask.SRV || !destination.RTV) {
                break;
            }

            SetSwapchainViewport();
            SetScissorRml(scissor);
            Diligent::OptimizedClearValue clearVal{};
            clearVal.Color[0] = clearVal.Color[1] = clearVal.Color[2] = clearVal.Color[3] = 0.0f;
            Diligent::ITextureView* pRTV = destination.RTV.RawPtr();
            m_Context->SetRenderTargets(1, &pRTV, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
            m_Context->ClearRenderTarget(pRTV, &clearVal, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
            BindPSO(m_PSO_BlendMask);
            Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> srb = GetOrCreateCachedSRB(
                m_PSO_BlendMask, source.SRV.RawPtr(), nullptr, [&](Diligent::IShaderResourceBinding* binding) {
                    if (auto* texVar = binding->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_InputTexture")) {
                        texVar->Set(source.SRV);
                    }
                    if (auto* maskVar = binding->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_MaskTexture")) {
                        maskVar->Set(blend_mask.SRV);
                    }
                    if (auto* sampVar = binding->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_SamplerLinear")) {
                        sampVar->Set(m_Sampler);
                    }
                },
                blend_mask.SRV.RawPtr());

            Diligent::ITextureView* pRTV1 = destination.RTV.RawPtr();
            m_Context->SetRenderTargets(1, &pRTV1, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
            Diligent::Uint64 offset = 0;
            Diligent::IBuffer* pVB = m_FullscreenVB;
            m_Context->SetVertexBuffers(0, 1, &pVB, &offset, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
            m_Context->SetIndexBuffer(m_FullscreenIB, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
            BindSRB(srb.RawPtr(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
            Diligent::DrawIndexedAttribs drawAttrs(6, Diligent::VT_UINT32, Diligent::DRAW_FLAG_NONE);
            m_Context->DrawIndexed(drawAttrs);
#ifdef RML_PERF_COUNTERS
            ++m_PerfCounters.drawIndexed;
#endif
            ++m_DrawCount;
            m_LayerStack.SwapPostprocessPrimarySecondary();
            ++m_FilterRenderCount;
        } break;

        case FilterType::Invalid:
        default:
            Rml::Log::Message(Rml::Log::LT_WARNING, "Unhandled render filter.");
            break;
        }
    }
}

Rml::CompiledFilterHandle RmlDiligentRenderInterface::CompileFilter(const Rml::String& name, const Rml::Dictionary& parameters)
{
    CompiledFilter filter{};

    if (name == "opacity") {
        filter.type = FilterType::Passthrough;
        filter.blend_factor = Rml::Get(parameters, "value", 1.0f);
    } else if (name == "blur") {
        filter.type = FilterType::Blur;
        filter.sigma = Rml::Get(parameters, "sigma", 1.0f);
    } else if (name == "drop-shadow") {
        filter.type = FilterType::DropShadow;
        filter.sigma = Rml::Get(parameters, "sigma", 0.f);
        filter.color = Rml::Get(parameters, "color", Rml::Colourb()).ToPremultiplied();
        filter.offset = Rml::Get(parameters, "offset", Rml::Vector2f(0.f));
    } else if (name == "brightness") {
        filter.type = FilterType::ColorMatrix;
        const float value = Rml::Get(parameters, "value", 1.0f);
        filter.color_matrix = Rml::Matrix4f::Diag(value, value, value, 1.f);
    } else if (name == "contrast") {
        filter.type = FilterType::ColorMatrix;
        const float value = Rml::Get(parameters, "value", 1.0f);
        const float grayness = 0.5f - 0.5f * value;
        filter.color_matrix = Rml::Matrix4f::Diag(value, value, value, 1.f);
        filter.color_matrix.SetColumn(3, Rml::Vector4f(grayness, grayness, grayness, 1.f));
    } else if (name == "invert") {
        filter.type = FilterType::ColorMatrix;
        const float value = Rml::Math::Clamp(Rml::Get(parameters, "value", 1.0f), 0.f, 1.f);
        const float inverted = 1.f - 2.f * value;
        filter.color_matrix = Rml::Matrix4f::Diag(inverted, inverted, inverted, 1.f);
        filter.color_matrix.SetColumn(3, Rml::Vector4f(value, value, value, 1.f));
    } else if (name == "grayscale") {
        filter.type = FilterType::ColorMatrix;
        const float value = Rml::Get(parameters, "value", 1.0f);
        const float rev_value = 1.f - value;
        const Rml::Vector3f gray = value * Rml::Vector3f(0.2126f, 0.7152f, 0.0722f);
        filter.color_matrix = Rml::Matrix4f::FromRows(
            {gray.x + rev_value, gray.y, gray.z, 0.f},
            {gray.x, gray.y + rev_value, gray.z, 0.f},
            {gray.x, gray.y, gray.z + rev_value, 0.f},
            {0.f, 0.f, 0.f, 1.f});
    } else if (name == "sepia") {
        filter.type = FilterType::ColorMatrix;
        const float value = Rml::Get(parameters, "value", 1.0f);
        const float rev_value = 1.f - value;
        const Rml::Vector3f r_mix = value * Rml::Vector3f(0.393f, 0.769f, 0.189f);
        const Rml::Vector3f g_mix = value * Rml::Vector3f(0.349f, 0.686f, 0.168f);
        const Rml::Vector3f b_mix = value * Rml::Vector3f(0.272f, 0.534f, 0.131f);
        filter.color_matrix = Rml::Matrix4f::FromRows(
            {r_mix.x + rev_value, r_mix.y, r_mix.z, 0.f},
            {g_mix.x, g_mix.y + rev_value, g_mix.z, 0.f},
            {b_mix.x, b_mix.y, b_mix.z + rev_value, 0.f},
            {0.f, 0.f, 0.f, 1.f});
    } else if (name == "hue-rotate") {
        filter.type = FilterType::ColorMatrix;
        const float value = Rml::Get(parameters, "value", 1.0f);
        const float s = Rml::Math::Sin(value);
        const float c = Rml::Math::Cos(value);
        filter.color_matrix = Rml::Matrix4f::FromRows(
            {0.213f + 0.787f * c - 0.213f * s, 0.715f - 0.715f * c - 0.715f * s, 0.072f - 0.072f * c + 0.928f * s, 0.f},
            {0.213f - 0.213f * c + 0.143f * s, 0.715f + 0.285f * c + 0.140f * s, 0.072f - 0.072f * c - 0.283f * s, 0.f},
            {0.213f - 0.213f * c - 0.787f * s, 0.715f - 0.715f * c + 0.715f * s, 0.072f + 0.928f * c + 0.072f * s, 0.f},
            {0.f, 0.f, 0.f, 1.f});
    } else if (name == "saturate") {
        filter.type = FilterType::ColorMatrix;
        const float value = Rml::Get(parameters, "value", 1.0f);
        filter.color_matrix = Rml::Matrix4f::FromRows(
            {0.213f + 0.787f * value, 0.715f - 0.715f * value, 0.072f - 0.072f * value, 0.f},
            {0.213f - 0.213f * value, 0.715f + 0.285f * value, 0.072f - 0.072f * value, 0.f},
            {0.213f - 0.213f * value, 0.715f - 0.715f * value, 0.072f + 0.928f * value, 0.f},
            {0.f, 0.f, 0.f, 1.f});
    }

    if (filter.type != FilterType::Invalid) {
        return reinterpret_cast<Rml::CompiledFilterHandle>(new CompiledFilter(filter));
    }

    Rml::Log::Message(Rml::Log::LT_WARNING, "Unsupported filter type '%s'.", name.c_str());
    return {};
}

void RmlDiligentRenderInterface::ReleaseFilter(Rml::CompiledFilterHandle filter)
{
    if (filter) {
        delete reinterpret_cast<CompiledFilter*>(filter);
    }
}

Rml::CompiledFilterHandle RmlDiligentRenderInterface::SaveLayerAsMaskImage()
{
    BlitLayerToPostprocessPrimary(m_LayerStack.GetTopLayerHandle());

    auto& source = m_LayerStack.GetPostprocessPrimary();
    auto& destination = m_LayerStack.GetBlendMask();
    if (!source.SRV || !destination.RTV) {
        return {};
    }

    //Diligent::OptimizedClearValue clearVal{};
    //clearVal.Color[0] = clearVal.Color[1] = clearVal.Color[2] = clearVal.Color[3] = 0.f;
    //if (auto* pRTV = destination.RTV.RawPtr()) {
    //    m_Context->SetRenderTargets(1, &pRTV, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    //    const bool previous_scissor_enabled = m_ScissorEnabled;
    //    m_ScissorEnabled = false;
    //    if (m_CachedWidth > 0 && m_CachedHeight > 0) {
    //        Diligent::Rect fullScissor{};
    //        fullScissor.left = 0;
    //        fullScissor.top = 0;
    //        fullScissor.right = m_CachedWidth;
    //        fullScissor.bottom = m_CachedHeight;
    //        m_Context->SetScissorRects(1, &fullScissor, static_cast<Diligent::Uint32>(m_CachedWidth),
    //            static_cast<Diligent::Uint32>(m_CachedHeight));
    //    }
    //    m_Context->ClearRenderTarget(pRTV, &clearVal, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    //    m_ScissorEnabled = previous_scissor_enabled;
    //}

    if (m_ScissorRegionRml.Valid()) {
        SetScissorRml(m_ScissorRegionRml);
    }
    DrawFullscreenPassthrough(source.SRV.RawPtr(), destination.RTV.RawPtr(), Rml::BlendMode::Replace, false,
        m_PSO_PassthroughReplace);
    BindTopLayer();

    auto* filter = new CompiledFilter{};
    filter->type = FilterType::MaskImage;
    return reinterpret_cast<Rml::CompiledFilterHandle>(filter);
}

Rml::TextureHandle RmlDiligentRenderInterface::SaveLayerAsTexture()
{
    if (!m_ScissorRegionRml.Valid()) {
        Rml::Log::Message(Rml::Log::LT_WARNING, "SaveLayerAsTexture: invalid scissor.");
        return {};
    }

    const Rml::Rectanglei bounds = m_ScissorRegionRml;
    const int width = bounds.Width();
    const int height = bounds.Height();
    if (width <= 0 || height <= 0) {
        return {};
    }

    BlitLayerToPostprocessPrimary(m_LayerStack.GetTopLayerHandle());

    auto& source = m_LayerStack.GetPostprocessPrimary();
    if (!source.SRV) {
        return {};
    }

    const bool previous_scissor_enabled = m_ScissorEnabled;
    constexpr int kLayerTextureGuardBand = 1;
    auto pooled = RenderTargetPool::AcquireColor(m_RTPool, width + 2 * kLayerTextureGuardBand, height + 2 * kLayerTextureGuardBand);
    if (!pooled || !pooled->texture) {
        m_ScissorEnabled = previous_scissor_enabled;
        if (previous_scissor_enabled && bounds.Valid()) {
            SetScissorRml(bounds, false);
        }
        return {};
    }

    UnbindRenderTargets();
    Diligent::OptimizedClearValue pooledClear{};
    pooledClear.Color[0] = pooledClear.Color[1] = pooledClear.Color[2] = pooledClear.Color[3] = 0.f;
    if (auto* pPooledRTV = pooled->RTV.RawPtr()) {
        m_Context->SetRenderTargets(1, &pPooledRTV, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        m_Context->ClearRenderTarget(pPooledRTV, &pooledClear, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    }

    Diligent::Box srcBox1{0, static_cast<Diligent::Uint32>(width), 0, static_cast<Diligent::Uint32>(height)};
    Diligent::CopyTextureAttribs copyAttribs;
    copyAttribs.pSrcTexture = source.texture;
    copyAttribs.pSrcBox = &srcBox1;
    copyAttribs.pDstTexture = pooled->texture;
    copyAttribs.DstX = static_cast<Diligent::Uint32>(kLayerTextureGuardBand);
    copyAttribs.DstY = static_cast<Diligent::Uint32>(kLayerTextureGuardBand);
    copyAttribs.SrcTextureTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
    copyAttribs.DstTextureTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
    m_Context->CopyTexture(copyAttribs);
    UnbindRenderTargets();

    // Match DX12: restore scissor to extracted bounds and rebind the top layer (with shared DSV).
    m_ScissorEnabled = previous_scissor_enabled;
    if (previous_scissor_enabled && bounds.Valid()) {
        SetScissorRegion(bounds);
    } else if (m_Context && m_SwapChain) {
        Diligent::Rect fullScissor{};
        fullScissor.left = 0;
        fullScissor.top = 0;
        fullScissor.right = static_cast<long>(m_SwapChain->GetDesc().Width);
        fullScissor.bottom = static_cast<long>(m_SwapChain->GetDesc().Height);
        m_Context->SetScissorRects(1, &fullScissor, static_cast<Diligent::Uint32>(m_SwapChain->GetDesc().Width),
            static_cast<Diligent::Uint32>(m_SwapChain->GetDesc().Height));
    }
    Diligent::Viewport fullVp{};
    fullVp.TopLeftX = 0.f;
    fullVp.TopLeftY = 0.f;
    fullVp.Width = static_cast<float>(m_SwapChain->GetDesc().Width);
    fullVp.Height = static_cast<float>(m_SwapChain->GetDesc().Height);
    fullVp.MinDepth = 0.f;
    fullVp.MaxDepth = 1.f;
    m_Context->SetViewports(1, &fullVp, static_cast<Diligent::Uint32>(m_SwapChain->GetDesc().Width),
        static_cast<Diligent::Uint32>(m_SwapChain->GetDesc().Height));
    BindTopLayer();

    auto* handle = new TextureHandle();
    handle->texture = pooled->texture;
    handle->SRV = pooled->SRV;
    handle->pooled = std::move(pooled);
    handle->fromPool = true;
    ++m_SaveLayerAsTextureCount;
    return reinterpret_cast<Rml::TextureHandle>(handle);
}

} // namespace RmlDiligent
