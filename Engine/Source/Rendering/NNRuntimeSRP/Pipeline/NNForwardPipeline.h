#pragma once

#include "INNRenderPipeline.h"
#include "NNPipelinePass.h"
#include "../Features/NNShadowPass.h"
#include "../Features/NNForwardPass.h"
#include "../Features/NNImGuiPass.h"
#include <NNRuntimeCore/NNObject.h>
#include <vector>

namespace NN::Runtime::SRP
{
    using namespace NN::Runtime::Core;

    // ========================================================================
    //  NNForwardPipeline â€?Default forward rendering pipeline
    //
    //  Pass order:
    //    1. Shadow Pass  â€?Render shadow maps from light POV
    //    2. Depth PrePass â€?Early Z for occlusion
    //    3. Forward Pass â€?Main scene rendering
    //    4. ImGui Pass   â€?Editor UI overlay
    // ========================================================================

    class NNForwardPipeline : public INNRenderPipeline
    {
    public:
        NNForwardPipeline();
        ~NNForwardPipeline() override = default;

        // === INNRenderPipeline ===
        bool Initialize(INNRenderDevice* device, uint32_t width, uint32_t height) override;
        void Execute(INNCommandList* cmd, const NNRenderContext& ctx) override;
        void Shutdown() override;
        const char* GetName() const override { return "ForwardPipeline"; }
        uint32_t GetPassCount() const override { return static_cast<uint32_t>(m_Passes.size()); }
        void OnResize(uint32_t width, uint32_t height) override;

        // === Access passes ===
        NNShadowPass& GetShadowPass() { return m_ShadowPass; }
        NNForwardPass& GetForwardPass() { return m_ForwardPass; }
        NNImGuiPass& GetImGuiPass() { return m_ImGuiPass; }

        // === INNObject ===
        uint32_t AddRef() override { return ++m_RefCount; }
        uint32_t Release() override { uint32_t r = --m_RefCount; if (r == 0) delete this; return r; }
        uint32_t GetRefCount() const override { return m_RefCount; }

    private:
        // Passes
        NNShadowPass m_ShadowPass;
        NNForwardPass m_ForwardPass;
        NNImGuiPass m_ImGuiPass;

        // Pass descriptors
        std::vector<NNPipelinePass> m_Passes;

        // State
        INNRenderDevice* m_Device = nullptr;
        uint32_t m_Width = 0;
        uint32_t m_Height = 0;
        std::atomic<uint32_t> m_RefCount{0};
    };

} // namespace NN::Runtime::SRP
