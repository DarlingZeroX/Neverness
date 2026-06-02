// NNDiligentPipelineState.cpp -- Diligent pipeline state wrapper implementation

#include "../../Pipeline/NNDiligentPipelineState.h"

namespace NNDiligent
{
    NNDiligentPipelineState::NNDiligentPipelineState(::Diligent::IPipelineState* pso,
                                                     ::Diligent::IRenderPass* renderPass,
                                                     const NNPipelineStateDesc& desc)
        : m_PSO(pso)
        , m_RenderPass(renderPass)
        , m_Desc(desc)
    {
    }

    NNDiligentPipelineState::~NNDiligentPipelineState()
    {
        if (m_RenderPass)
        {
            m_RenderPass->Release();
            m_RenderPass = nullptr;
        }
        if (m_PSO)
        {
            m_PSO->Release();
            m_PSO = nullptr;
        }
    }

    const NNPipelineStateDesc& NNDiligentPipelineState::GetDesc() const { return m_Desc; }

    uint32_t NNDiligentPipelineState::AddRef() { return ++m_RefCount; }
    uint32_t NNDiligentPipelineState::Release()
    {
        uint32_t c = --m_RefCount;
        if (c == 0) delete this;
        return c;
    }
    uint32_t NNDiligentPipelineState::GetRefCount() const { return m_RefCount; }

} // namespace NNDiligent
