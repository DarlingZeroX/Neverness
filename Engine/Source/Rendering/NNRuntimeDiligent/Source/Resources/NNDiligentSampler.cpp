// NNDiligentSampler.cpp — Diligent Sampler wrapper

#include "../../Resources/NNDiligentSampler.h"

namespace NNDiligent
{
    NNDiligentSampler::NNDiligentSampler(::Diligent::ISampler* sampler, const NNSamplerDesc& desc)
        : m_Sampler(sampler)
        , m_Desc(desc)
    {
        if (m_Sampler) m_Sampler->AddRef();
    }

    NNDiligentSampler::~NNDiligentSampler()
    {
        if (m_Sampler)
        {
            m_Sampler->Release();
            m_Sampler = nullptr;
        }
    }

    const NNSamplerDesc& NNDiligentSampler::GetDesc() const
    {
        return m_Desc;
    }

    uint32_t NNDiligentSampler::AddRef()
    {
        return ++m_RefCount;
    }

    uint32_t NNDiligentSampler::Release()
    {
        uint32_t c = --m_RefCount;
        if (c == 0)
            delete this;
        return c;
    }

    uint32_t NNDiligentSampler::GetRefCount() const
    {
        return m_RefCount;
    }

} // namespace NNDiligent
