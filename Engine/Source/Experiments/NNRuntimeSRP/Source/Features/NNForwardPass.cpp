// NNForwardPass.cpp — Forward pass implementation

#include "../../Features/NNForwardPass.h"
#include <iostream>

namespace NN::Runtime::SRP
{
    bool NNForwardPass::Setup(INNRenderDevice* device, uint32_t width, uint32_t height,
                               NNPixelFormat colorFormat, NNPixelFormat depthFormat)
    {
        if (!device) return false;

        m_Width = width;
        m_Height = height;
        m_ColorFormat = colorFormat;
        m_DepthFormat = depthFormat;

        // Create color render target
        NNRenderTargetDesc colorDesc;
        colorDesc.Width = width;
        colorDesc.Height = height;
        colorDesc.ColorFormat = colorFormat;
        colorDesc.DepthFormat = NNPixelFormat::Unknown; // Separate depth
        colorDesc.DebugName = "ForwardColor";

        m_ColorRT = device->CreateRenderTarget(colorDesc);
        if (!m_ColorRT)
        {
            std::cerr << "[ForwardPass] Failed to create color RT" << std::endl;
            return false;
        }

        // Create depth render target
        NNRenderTargetDesc depthDesc;
        depthDesc.Width = width;
        depthDesc.Height = height;
        depthDesc.ColorFormat = NNPixelFormat::Unknown; // Depth-only
        depthDesc.DepthFormat = depthFormat;
        depthDesc.DebugName = "ForwardDepth";

        m_DepthRT = device->CreateRenderTarget(depthDesc);
        if (!m_DepthRT)
        {
            std::cerr << "[ForwardPass] Failed to create depth RT" << std::endl;
            return false;
        }

        std::cout << "[ForwardPass] Render targets created: " << width << "x" << height << std::endl;
        return true;
    }

    void NNForwardPass::Execute(INNCommandList* cmd, const NNRenderContext& ctx,
                                 const std::vector<NNForwardDrawCall>& drawCalls)
    {
        if (!cmd || !m_ColorRT) return;

        // Forward pass execution:
        // In production, this would:
        //   1. Bind color + depth render targets
        //   2. Clear color and depth
        //   3. Set viewport to frame size
        //   4. For each draw call:
        //      a. Apply material (sets PSO + bindings)
        //      b. Set vertex/index buffers
        //      c. Draw
        //
        // The actual render commands use INNCommandList interface:
        //   SetViewports, SetScissorRects, SetPipelineState,
        //   SetVertexBuffer, SetIndexBuffer, Draw, DrawIndexed
        //
        // BeginRenderPass/EndRenderPass are Diligent-specific
        // and handled by the backend implementation.

        for (const auto& dc : drawCalls)
        {
            if (!dc.Material) continue;

            // Apply material (sets PSO)
            if (ctx.PipelineCache)
            {
                NNVertexLayout layout;
                layout.Stride = 32;
                dc.Material->Apply(cmd, ctx.PipelineCache, layout,
                                   m_ColorFormat, m_DepthFormat);
            }

            if (dc.VertexBuffer)
            {
                cmd->SetVertexBuffer(dc.VertexBuffer);
            }

            if (dc.IndexBuffer)
            {
                cmd->SetIndexBuffer(dc.IndexBuffer);
            }

            if (dc.IndexCount > 0)
            {
                NNDrawIndexedAttribs draw;
                draw.IndexCount = dc.IndexCount;
                cmd->DrawIndexed(draw);
            }
            else if (dc.VertexCount > 0)
            {
                NNDrawAttribs draw;
                draw.VertexCount = dc.VertexCount;
                cmd->Draw(draw);
            }
        }
    }

    void NNForwardPass::OnResize(INNRenderDevice* device, uint32_t width, uint32_t height)
    {
        if (width == m_Width && height == m_Height) return;
        Setup(device, width, height, m_ColorFormat, m_DepthFormat);
    }

} // namespace NN::Runtime::SRP
