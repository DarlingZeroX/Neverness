// NNPipelinePass.cpp — Render pass implementation

#include "../../Pipeline/NNPipelinePass.h"
#include <iostream>

namespace NN::Runtime::SRP
{
    NNPipelinePass::NNPipelinePass(const NNPipelinePassDesc& desc)
        : m_Desc(desc)
    {
    }

    bool NNPipelinePass::SetupTarget(INNRenderDevice* device, uint32_t frameW, uint32_t frameH)
    {
        if (!device) return false;

        uint32_t w = m_Desc.Width > 0 ? m_Desc.Width : frameW;
        uint32_t h = m_Desc.Height > 0 ? m_Desc.Height : frameH;

        // Skip if already correct size
        if (m_RenderTarget && m_RenderTarget->GetWidth() == w && m_RenderTarget->GetHeight() == h)
        {
            return true;
        }

        // Create render target
        NNRenderTargetDesc rtDesc;
        rtDesc.Width = w;
        rtDesc.Height = h;
        rtDesc.ColorFormat = m_Desc.RTVFormat;
        rtDesc.DepthFormat = m_Desc.DSVFormat;
        rtDesc.DebugName = m_Desc.Name.c_str();

        m_RenderTarget = device->CreateRenderTarget(rtDesc);
        if (!m_RenderTarget)
        {
            std::cerr << "[NNPipelinePass] Failed to create RT: " << m_Desc.Name << std::endl;
            return false;
        }

        return true;
    }

    void NNPipelinePass::Begin(INNCommandList* cmd)
    {
        if (!cmd || !m_RenderTarget) return;

        // Begin pass:
        // In production, this would:
        //   1. Bind the render target (handled by backend)
        //   2. Set viewport to render target size
        //   3. Set scissor rect
        //   4. Clear color/depth if configured
        //
        // The actual render commands use INNCommandList interface:
        //   SetViewports, SetScissorRects
        //
        // BeginRenderPass/EndRenderPass are Diligent-specific
        // and handled by the backend implementation.

        float w = static_cast<float>(GetActualWidth());
        float h = static_cast<float>(GetActualHeight());
        NNViewport vp = {0, 0, w, h, 0.0f, 1.0f};
        cmd->SetViewports(&vp, 1);

        NNRect scissor = {0, 0, static_cast<int32_t>(w), static_cast<int32_t>(h)};
        cmd->SetScissorRects(&scissor, 1);
    }

    void NNPipelinePass::End(INNCommandList* cmd)
    {
        // End pass:
        // In production, this would:
        //   1. Resolve MSAA if needed
        //   2. Transition render target for sampling
        // These are backend-specific operations.
    }

    uint32_t NNPipelinePass::GetActualWidth() const
    {
        if (m_RenderTarget) return m_RenderTarget->GetWidth();
        return m_Desc.Width;
    }

    uint32_t NNPipelinePass::GetActualHeight() const
    {
        if (m_RenderTarget) return m_RenderTarget->GetHeight();
        return m_Desc.Height;
    }

} // namespace NN::Runtime::SRP
