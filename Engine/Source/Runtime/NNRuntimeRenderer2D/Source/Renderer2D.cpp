/**
 * @file Renderer2D.cpp
 * @brief 2D 渲染器实现：Diligent 后端。
 *
 * 资源创建通过 INNRenderDevice（NNRuntimeRender 接口）。
 * 渲染循环通过 raw Diligent IDeviceContext（因 NNRuntimeRender 缺少 SRB 绑定）。
 * 着色器为 HLSL（NNRuntimeRender 的 CreateShader 目前只支持 HLSL）。
 */

#include "Renderer2D/Renderer2D.h"
#include "Renderer2D/BuiltinShaders.h"

// NNRuntimeRender 接口
#include <Resources/INNBuffer.h>
#include <Resources/INNTexture.h>
#include <Resources/INNSampler.h>
#include <Pipeline/INNPipelineState.h>
#include <Pipeline/INNShader.h>
#include <Device/INNRenderDevice.h>

// NNDiligent 内部（用于 raw Diligent 操作）
#include "NNDiligentConfig.h"
#include "Device/NNDiligentDevice.h"
#include "Resources/NNDiligentBuffer.h"
#include "Resources/NNDiligentTexture.h"
#include "Resources/NNDiligentSampler.h"
#include "Pipeline/NNDiligentPipelineState.h"
#include "Pipeline/NNDiligentShader.h"

// Diligent MapHelper（常量缓冲区 Map/Unmap）
#include "Graphics/GraphicsTools/interface/MapHelper.hpp"

#include <cstring>
#include <unordered_map>

using namespace Diligent;

namespace NN::Runtime::Renderer2D
{
    // ========================================================================
    //  常量缓冲区布局（与 HLSL cbuffer 对齐）
    // ========================================================================
    struct SpriteCB
    {
        float ViewProjection[16]; // float4x4, 64 bytes
        float Transform[16];      // float4x4, 64 bytes
        float UvRect[4];          // float4,    16 bytes
        float Color[4];           // float4,    16 bytes
        int   FlipX;              // int,        4 bytes
        int   FlipY;              // int,        4 bytes
        int   Padding[2];         // 对齐到 16 字节边界
    };
    static_assert(sizeof(SpriteCB) == 176, "SpriteCB size mismatch");

    // ========================================================================
    //  Unit Quad 顶点数据
    // ========================================================================
    static constexpr float QuadVertices[] = {
        // x      y      z     u     v
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,  // 左下
         0.5f, -0.5f, 0.0f, 1.0f, 0.0f,  // 右下
         0.5f,  0.5f, 0.0f, 1.0f, 1.0f,  // 右上
        -0.5f,  0.5f, 0.0f, 0.0f, 1.0f,  // 左上
    };

    static constexpr unsigned int QuadIndices[] = {
        0, 1, 2,
        2, 3, 0,
    };

    // ========================================================================
    //  Renderer2D::Impl
    // ========================================================================
    struct Renderer2D::Impl
    {
        // Diligent 原始指针（从 NNDiligentDevice 获取）
        IRenderDevice*  diliDevice  = nullptr;
        IDeviceContext* diliContext = nullptr;

        // NNRuntimeRender 资源（用于生命周期管理）
        Core::NNRef<Render::INNBuffer>   VB;
        Core::NNRef<Render::INNBuffer>   IB;
        Core::NNRef<Render::INNBuffer>   CB;
        Core::NNRef<Render::INNTexture>  WhiteTexture;
        Core::NNRef<Render::INNSampler>  Sampler;

        // Diligent 缓存指针（从 NNRuntimeRender 包装器提取，用于渲染循环）
        IPipelineState*              diliPSO        = nullptr;
        IRenderPass*                 diliRenderPass = nullptr;
        ITextureView*                whiteSRV       = nullptr;
        ISampler*                    diliSampler    = nullptr;
        IBuffer*                     diliCB         = nullptr;

        // SRB 缓存：textureView* → SRB
        std::unordered_map<void*, RefCntAutoPtr<IShaderResourceBinding>> SRBCache;

        // 缓存的 ViewProjection 矩阵（BeginScene 设置，Submit 中每个 CB 更新使用）
        float CachedVP[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};

        // 统计
        std::uint32_t DrawCallCount = 0;
        std::uint32_t QuadCount     = 0;
    };

    // ========================================================================
    //  Renderer2D 公开接口
    // ========================================================================

    Renderer2D::Renderer2D() : m_Impl(nullptr) {}
    Renderer2D::~Renderer2D() { Shutdown(); }

    bool Renderer2D::Initialize(Render::INNRenderDevice* device)
    {
        if (!device)
            return false;

        m_Impl = new Impl();

        // 获取 Diligent 原始指针
        auto* dilDev = static_cast<NNDiligent::NNDiligentDevice*>(device);
        m_Impl->diliDevice  = dilDev->GetDiligentDevice();
        m_Impl->diliContext = dilDev->GetDiligentContext();

        if (!m_Impl->diliDevice || !m_Impl->diliContext)
        {
            delete m_Impl;
            m_Impl = nullptr;
            return false;
        }

        // ── 1. 创建着色器 ──
        Render::NNShaderDesc vsDesc{};
        vsDesc.Stage       = Render::NNShaderStage::Vertex;
        vsDesc.Language    = Render::NNShaderLanguage::HLSL;
        vsDesc.SourceCode  = BuiltinShaders::SpriteVS;
        vsDesc.EntryPoint  = "main";
        vsDesc.DebugName   = "SpriteVS";

        Render::NNShaderDesc psDesc{};
        psDesc.Stage       = Render::NNShaderStage::Pixel;
        psDesc.Language    = Render::NNShaderLanguage::HLSL;
        psDesc.SourceCode  = BuiltinShaders::SpriteFS;
        psDesc.EntryPoint  = "main";
        psDesc.DebugName   = "SpritePS";

        auto vs = device->CreateShader(vsDesc);
        auto ps = device->CreateShader(psDesc);
        if (!vs || !ps)
        {
            delete m_Impl;
            m_Impl = nullptr;
            return false;
        }

        auto* diliVS = static_cast<NNDiligent::NNDiligentShader*>(vs.Get())->GetDiligentShader();
        auto* diliPS = static_cast<NNDiligent::NNDiligentShader*>(ps.Get())->GetDiligentShader();

        // ── 2. 创建 RenderPass ──
        RenderPassAttachmentDesc rtAttach{};
        rtAttach.Format          = TEX_FORMAT_RGBA8_UNORM;
        rtAttach.LoadOp          = ATTACHMENT_LOAD_OP_CLEAR;
        rtAttach.StoreOp         = ATTACHMENT_STORE_OP_STORE;
        rtAttach.StencilLoadOp   = ATTACHMENT_LOAD_OP_DISCARD;
        rtAttach.StencilStoreOp  = ATTACHMENT_STORE_OP_DISCARD;
        rtAttach.InitialState    = RESOURCE_STATE_RENDER_TARGET;
        rtAttach.FinalState      = RESOURCE_STATE_RENDER_TARGET;

        RenderPassAttachmentDesc dsAttach{};
        dsAttach.Format          = TEX_FORMAT_D24_UNORM_S8_UINT;
        dsAttach.LoadOp          = ATTACHMENT_LOAD_OP_CLEAR;
        dsAttach.StoreOp         = ATTACHMENT_STORE_OP_DISCARD;
        dsAttach.StencilLoadOp   = ATTACHMENT_LOAD_OP_CLEAR;
        dsAttach.StencilStoreOp  = ATTACHMENT_STORE_OP_DISCARD;
        dsAttach.InitialState    = RESOURCE_STATE_DEPTH_WRITE;
        dsAttach.FinalState      = RESOURCE_STATE_DEPTH_WRITE;

        RenderPassAttachmentDesc attachments[] = { rtAttach, dsAttach };

        RenderPassDesc RPDesc{};
        RPDesc.Name            = "SpriteRP";
        RPDesc.AttachmentCount = 2;
        RPDesc.pAttachments    = attachments;

        RefCntAutoPtr<IRenderPass> renderPass;
        m_Impl->diliDevice->CreateRenderPass(RPDesc, &renderPass);
        if (!renderPass)
        {
            delete m_Impl;
            m_Impl = nullptr;
            return false;
        }
        m_Impl->diliRenderPass = renderPass;

        // ── 3. 创建 PSO（raw Diligent，控制变量类型为 MUTABLE） ──
        GraphicsPipelineStateCreateInfo psoCI{};
        psoCI.PSODesc.Name                     = "SpritePSO";
        psoCI.PSODesc.PipelineType             = PIPELINE_TYPE_GRAPHICS;
        psoCI.PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE;

        psoCI.GraphicsPipeline.pRenderPass       = renderPass;
        psoCI.GraphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        // 顶点布局：Position(float3, offset 0) + TexCoord(float2, offset 12)
        LayoutElement layoutElems[] = {
            LayoutElement{0, 0, 3, VT_FLOAT32, false, 0},
            LayoutElement{1, 0, 2, VT_FLOAT32, false, 12},
        };
        psoCI.GraphicsPipeline.InputLayout.LayoutElements = layoutElems;
        psoCI.GraphicsPipeline.InputLayout.NumElements     = 2;

        // 光栅化：无面剔除（2D）
        psoCI.GraphicsPipeline.RasterizerDesc.CullMode = CULL_MODE_NONE;
        psoCI.GraphicsPipeline.RasterizerDesc.FillMode = FILL_MODE_SOLID;

        // 混合：Alpha 混合
        psoCI.GraphicsPipeline.BlendDesc.RenderTargets[0].BlendEnable    = True;
        psoCI.GraphicsPipeline.BlendDesc.RenderTargets[0].SrcBlend       = BLEND_FACTOR_SRC_ALPHA;
        psoCI.GraphicsPipeline.BlendDesc.RenderTargets[0].DestBlend      = BLEND_FACTOR_INV_SRC_ALPHA;
        psoCI.GraphicsPipeline.BlendDesc.RenderTargets[0].BlendOp        = BLEND_OPERATION_ADD;
        psoCI.GraphicsPipeline.BlendDesc.RenderTargets[0].SrcBlendAlpha  = BLEND_FACTOR_ONE;
        psoCI.GraphicsPipeline.BlendDesc.RenderTargets[0].DestBlendAlpha = BLEND_FACTOR_INV_SRC_ALPHA;
        psoCI.GraphicsPipeline.BlendDesc.RenderTargets[0].BlendOpAlpha   = BLEND_OPERATION_ADD;

        // 深度：禁用（2D）
        psoCI.GraphicsPipeline.DepthStencilDesc.DepthEnable      = False;
        psoCI.GraphicsPipeline.DepthStencilDesc.DepthWriteEnable = False;

        // 着色器
        psoCI.pVS = diliVS;
        psoCI.pPS = diliPS;

        RefCntAutoPtr<IPipelineState> pso;
        m_Impl->diliDevice->CreatePipelineState(psoCI, &pso);
        if (!pso)
        {
            delete m_Impl;
            m_Impl = nullptr;
            return false;
        }
        m_Impl->diliPSO = pso;

        // ── 4. 创建顶点缓冲（Static） ──
        Render::NNBufferDesc vbDesc{};
        vbDesc.Type  = Render::NNBufferType::Vertex;
        vbDesc.Usage = Render::NNBufferUsage::Static;
        vbDesc.Size  = sizeof(QuadVertices);
        m_Impl->VB = device->CreateBuffer(vbDesc, QuadVertices);
        if (!m_Impl->VB)
        {
            delete m_Impl;
            m_Impl = nullptr;
            return false;
        }

        // ── 5. 创建索引缓冲（Static） ──
        Render::NNBufferDesc ibDesc{};
        ibDesc.Type  = Render::NNBufferType::Index;
        ibDesc.Usage = Render::NNBufferUsage::Static;
        ibDesc.Size  = sizeof(QuadIndices);
        m_Impl->IB = device->CreateBuffer(ibDesc, QuadIndices);
        if (!m_Impl->IB)
        {
            delete m_Impl;
            m_Impl = nullptr;
            return false;
        }

        // ── 6. 创建常量缓冲（Dynamic，每帧更新） ──
        Render::NNBufferDesc cbDesc{};
        cbDesc.Type  = Render::NNBufferType::Constant;
        cbDesc.Usage = Render::NNBufferUsage::Dynamic;
        cbDesc.Size  = sizeof(SpriteCB);
        m_Impl->CB = device->CreateBuffer(cbDesc, nullptr);
        if (!m_Impl->CB)
        {
            delete m_Impl;
            m_Impl = nullptr;
            return false;
        }
        // 提取 CB 指针（用于 SRB 绑定 — SRB 的 Set(IBuffer*) 接受 buffer 对象）
        m_Impl->diliCB = static_cast<NNDiligent::NNDiligentBuffer*>(m_Impl->CB.Get())->GetDiligentBuffer();

        // ── 7. 创建 1x1 白色纹理 ──
        Render::NNTextureDesc texDesc{};
        texDesc.Width  = 1;
        texDesc.Height = 1;
        texDesc.Format = Render::NNPixelFormat::RGBA8_UNORM;
        const std::uint32_t white = 0xFFFFFFFF;
        m_Impl->WhiteTexture = device->CreateTexture(texDesc, &white);
        if (!m_Impl->WhiteTexture)
        {
            delete m_Impl;
            m_Impl = nullptr;
            return false;
        }
        m_Impl->whiteSRV = static_cast<NNDiligent::NNDiligentTexture*>(
            m_Impl->WhiteTexture.Get())->GetDiligentTexture()->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);

        // ── 8. 创建采样器（Nearest, Clamp） ──
        Render::NNSamplerDesc sampDesc{};
        sampDesc.MinFilter = Render::NNFilterMode::Point;
        sampDesc.MagFilter = Render::NNFilterMode::Point;
        sampDesc.AddressU  = Render::NNAddressMode::Clamp;
        sampDesc.AddressV  = Render::NNAddressMode::Clamp;
        m_Impl->Sampler = device->CreateSampler(sampDesc);
        if (!m_Impl->Sampler)
        {
            delete m_Impl;
            m_Impl = nullptr;
            return false;
        }
        m_Impl->diliSampler = static_cast<NNDiligent::NNDiligentSampler*>(
            m_Impl->Sampler.Get())->GetDiligentSampler();

        return true;
    }

    void Renderer2D::Shutdown()
    {
        if (m_Impl)
        {
            m_Impl->SRBCache.clear();
            m_Impl->CB.Reset();
            m_Impl->VB.Reset();
            m_Impl->IB.Reset();
            m_Impl->WhiteTexture.Reset();
            m_Impl->Sampler.Reset();
            delete m_Impl;
            m_Impl = nullptr;
        }
    }

    void Renderer2D::BeginScene(const CameraData& camera, std::uint32_t width, std::uint32_t height)
    {
        if (!m_Impl) return;

        m_Impl->DrawCallCount = 0;
        m_Impl->QuadCount     = 0;

        // 缓存 ViewProjection 矩阵（Submit 中每个 CB 更新使用）
        std::memcpy(m_Impl->CachedVP, camera.ViewProjectionMatrix, sizeof(float) * 16);

        // 设置视口
        Viewport vp{};
        vp.Width    = static_cast<float>(width);
        vp.Height   = static_cast<float>(height);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        m_Impl->diliContext->SetViewports(1, &vp, 0, 0);
    }

    void Renderer2D::Submit(const std::vector<SpriteDrawCommand>& commands)
    {
        if (!m_Impl || !m_Impl->diliPSO) return;

        // 设置 PSO 和 VB/IB（整个 Submit 共享）
        m_Impl->diliContext->SetPipelineState(m_Impl->diliPSO);

        auto* diliVB = static_cast<NNDiligent::NNDiligentBuffer*>(m_Impl->VB.Get())->GetDiligentBuffer();
        auto* diliIB = static_cast<NNDiligent::NNDiligentBuffer*>(m_Impl->IB.Get())->GetDiligentBuffer();
        IBuffer* pVBs[] = { diliVB };
        Uint64 offsets[] = { 0 };
        m_Impl->diliContext->SetVertexBuffers(0, 1, pVBs, offsets, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);
        m_Impl->diliContext->SetIndexBuffer(diliIB, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        for (const auto& cmd : commands)
        {
            // ── 更新常量缓冲（Map/Unmap） ──
            {
                MapHelper<SpriteCB> mappedCB(m_Impl->diliContext, m_Impl->diliCB, MAP_WRITE, MAP_FLAG_DISCARD);
                if (!mappedCB) continue;

                std::memcpy(mappedCB->ViewProjection, m_Impl->CachedVP, sizeof(float) * 16);
                std::memcpy(mappedCB->Transform, cmd.Transform, sizeof(float) * 16);
                std::memcpy(mappedCB->UvRect, cmd.UvRect, sizeof(float) * 4);
                std::memcpy(mappedCB->Color, cmd.Color, sizeof(float) * 4);
                mappedCB->FlipX   = cmd.FlipX ? 1 : 0;
                mappedCB->FlipY   = cmd.FlipY ? 1 : 0;
                mappedCB->Padding[0] = 0;
                mappedCB->Padding[1] = 0;
            }

            // ── 获取纹理 SRV ──
            ITextureView* texSRV = (cmd.TextureHandle == 0)
                ? m_Impl->whiteSRV
                : reinterpret_cast<ITextureView*>(cmd.TextureHandle);

            if (!texSRV)
                texSRV = m_Impl->whiteSRV;

            // ── SRB 缓存查找/创建 ──
            auto it = m_Impl->SRBCache.find(texSRV);
            if (it == m_Impl->SRBCache.end())
            {
                RefCntAutoPtr<IShaderResourceBinding> srb;
                m_Impl->diliPSO->CreateShaderResourceBinding(&srb, true);
                if (srb)
                {
                    // 绑定 CB（VS 和 PS 都引用 SpriteConstants）
                    auto* varVS = srb->GetVariableByName(SHADER_TYPE_VERTEX, "SpriteConstants");
                    if (varVS) varVS->Set(m_Impl->diliCB);
                    auto* varPS = srb->GetVariableByName(SHADER_TYPE_PIXEL, "SpriteConstants");
                    if (varPS) varPS->Set(m_Impl->diliCB);

                    // 绑定纹理
                    auto* texVar = srb->GetVariableByName(SHADER_TYPE_PIXEL, "u_Texture");
                    if (texVar) texVar->Set(texSRV);

                    // 绑定采样器
                    auto* sampVar = srb->GetVariableByName(SHADER_TYPE_PIXEL, "u_Sampler");
                    if (sampVar) sampVar->Set(m_Impl->diliSampler);
                }
                it = m_Impl->SRBCache.emplace(texSRV, std::move(srb)).first;
            }

            // ── 提交 SRB 并绘制 ──
            if (it->second)
            {
                m_Impl->diliContext->CommitShaderResources(it->second, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
            }

            DrawIndexedAttribs drawAttrs;
            drawAttrs.IndexType  = VT_UINT32;
            drawAttrs.NumIndices = 6;
            drawAttrs.Flags      = DRAW_FLAG_VERIFY_ALL;
            m_Impl->diliContext->DrawIndexed(drawAttrs);

            m_Impl->DrawCallCount++;
            m_Impl->QuadCount++;
        }
    }

    void Renderer2D::EndScene()
    {
        // Diligent immediate context 是同步的，无需显式 flush
    }

    std::uint32_t Renderer2D::GetDrawCallCount() const
    {
        return m_Impl ? m_Impl->DrawCallCount : 0;
    }

    std::uint32_t Renderer2D::GetQuadCount() const
    {
        return m_Impl ? m_Impl->QuadCount : 0;
    }
}
