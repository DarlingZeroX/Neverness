#pragma once

#include "../Context/NNRenderContext.h"
#include "../Pipeline/NNPipelinePass.h"
#include <NNRuntimeRender/Command/INNCommandList.h>
#include <NNRuntimeRender/RenderTarget/INNRenderTarget.h>
#include <NNRuntimeRenderAssets/Material/NNMaterial.h>
#include <NNRuntimeCore/NNObject.h>
#include <vector>

namespace NN::Runtime::SRP
{
    using namespace NN::Runtime::Render;
    using namespace NN::Runtime::Core;
    using namespace NN::Runtime::Assets;

    // ========================================================================
    //  Draw call data for forward pass
    // ========================================================================

    struct NNForwardDrawCall
    {
        NNMaterial* Material = nullptr;  // Material to apply
        INNBuffer* VertexBuffer = nullptr;
        INNBuffer* IndexBuffer = nullptr;
        uint32_t IndexCount = 0;
        uint32_t VertexCount = 0;
        NNPrimitiveTopology Topology = NNPrimitiveTopology::TriangleList;
    };

    // ========================================================================
    //  NNForwardPass â€?Main scene rendering pass
    //  Draws all opaque and transparent objects with materials
    // ========================================================================

    class NNForwardPass
    {
    public:
        NNForwardPass() = default;
        ~NNForwardPass() = default;

        // Setup render target (color + depth)
        bool Setup(INNRenderDevice* device, uint32_t width, uint32_t height,
                   NNPixelFormat colorFormat = NNPixelFormat::RGBA8_UNORM,
                   NNPixelFormat depthFormat = NNPixelFormat::D32_FLOAT);

        // Execute forward pass
        void Execute(INNCommandList* cmd, const NNRenderContext& ctx,
                     const std::vector<NNForwardDrawCall>& drawCalls);

        // Get render targets
        NNRef<INNRenderTarget> GetColorTarget() const { return m_ColorRT; }
        NNRef<INNRenderTarget> GetDepthTarget() const { return m_DepthRT; }

        // Resize
        void OnResize(INNRenderDevice* device, uint32_t width, uint32_t height);

    private:
        NNRef<INNRenderTarget> m_ColorRT;
        NNRef<INNRenderTarget> m_DepthRT;
        NNPixelFormat m_ColorFormat = NNPixelFormat::RGBA8_UNORM;
        NNPixelFormat m_DepthFormat = NNPixelFormat::D32_FLOAT;
        uint32_t m_Width = 0;
        uint32_t m_Height = 0;
    };

} // namespace NN::Runtime::SRP
