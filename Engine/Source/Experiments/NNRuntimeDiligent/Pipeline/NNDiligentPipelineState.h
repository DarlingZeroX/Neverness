// NNDiligentPipelineState.h -- Diligent pipeline state wrapper

#pragma once

#include "../NNDiligentConfig.h"
#include <NNRuntimeRender/Pipeline/INNPipelineState.h>
#include <NNRuntimeCore/NNObject.h>
#include <atomic>

namespace NNDiligent
{
    using namespace NN::Runtime::Core;
    using namespace NN::Runtime::Render;

    class NNDiligentPipelineState : public INNPipelineState
    {
    public:
        NNDiligentPipelineState(::Diligent::IPipelineState* pso,
                                ::Diligent::IRenderPass* renderPass,
                                const NNPipelineStateDesc& desc);
        ~NNDiligentPipelineState() override;

        // INNPipelineState
        const NNPipelineStateDesc& GetDesc() const override;

        // INNObject
        uint32_t AddRef() override;
        uint32_t Release() override;
        uint32_t GetRefCount() const override;

        // Internal: get the underlying Diligent PSO
        ::Diligent::IPipelineState* GetDiligentPSO() const { return m_PSO; }
        // Internal: get the render pass used by this PSO
        ::Diligent::IRenderPass* GetRenderPass() const { return m_RenderPass; }

    private:
        ::Diligent::IPipelineState* m_PSO = nullptr;
        ::Diligent::IRenderPass*    m_RenderPass = nullptr;
        NNPipelineStateDesc m_Desc;
        std::atomic<uint32_t> m_RefCount{0};
    };

} // namespace NNDiligent
