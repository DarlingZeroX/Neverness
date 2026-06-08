#pragma once

#include "../NNRenderConfig.h"
#include <NNRuntimeRender/Resources/INNTexture.h>
#include <NNRuntimeRender/Command/INNCommandList.h>
#include <NNRuntimeRender/Device/INNRenderDevice.h>
#include <NNRuntimeRender/RenderTarget/INNRenderTarget.h>
#include <NNRuntimeCore/NNObject.h>
#include <cstdint>
#include <string>

namespace NN::Runtime::SRP
{
    using namespace NN::Runtime::Render;
    using namespace NN::Runtime::Core;

    // ========================================================================
    //  Pass types in a typical pipeline
    // ========================================================================

    enum class NNPassType : uint8_t
    {
        Shadow,         // Shadow map generation
        DepthPrePass,   // Early depth pass
        Forward,        // Forward rendering
        Deferred_GBuf,  // Deferred: G-Buffer fill
        Deferred_Light, // Deferred: Lighting
        PostProcess,    // Post-processing
        ImGui,          // ImGui overlay
        Custom          // User-defined
    };

    // ========================================================================
    //  NNPipelinePassDesc �?Description of a render pass
    // ========================================================================

    struct NNPipelinePassDesc
    {
        std::string Name;
        NNPassType Type = NNPassType::Forward;

        // Render target format
        NNPixelFormat RTVFormat = NNPixelFormat::RGBA8_UNORM;
        NNPixelFormat DSVFormat = NNPixelFormat::D32_FLOAT;

        // Dimensions (0 = use frame size)
        uint32_t Width = 0;
        uint32_t Height = 0;

        // Clear settings
        bool ClearColor = true;
        bool ClearDepth = true;
        float ClearR = 0.0f, ClearG = 0.0f, ClearB = 0.0f, ClearA = 1.0f;

        // Viewport (0 = use full target)
        float ViewportX = 0, ViewportY = 0;
        float ViewportW = 0, ViewportH = 0;
    };

    // ========================================================================
    //  NNPipelinePass �?A single render pass in the pipeline
    // ========================================================================

    class NNPipelinePass
    {
    public:
        NNPipelinePass() = default;
        NNPipelinePass(const NNPipelinePassDesc& desc);
        ~NNPipelinePass() = default;

        // Setup render target (called during pipeline init)
        bool SetupTarget(INNRenderDevice* device, uint32_t frameW, uint32_t frameH);

        // Begin pass (set RT, clear)
        void Begin(INNCommandList* cmd);

        // End pass (resolve, transition)
        void End(INNCommandList* cmd);

        // Getters
        const NNPipelinePassDesc& GetDesc() const { return m_Desc; }
        NNRef<INNRenderTarget> GetRenderTarget() const { return m_RenderTarget; }
        uint32_t GetActualWidth() const;
        uint32_t GetActualHeight() const;

    private:
        NNPipelinePassDesc m_Desc;
        NNRef<INNRenderTarget> m_RenderTarget;
    };

} // namespace NN::Runtime::SRP
