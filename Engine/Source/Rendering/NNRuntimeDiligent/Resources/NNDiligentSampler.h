#pragma once

#include "../NNDiligentConfig.h"
#include <NNRuntimeRender/Resources/INNSampler.h>
#include <NNRuntimeCore/NNObject.h>
#include <atomic>

namespace NNDiligent
{
    using namespace NN::Runtime::Core;
    using namespace NN::Runtime::Render;

    class NNDiligentSampler : public INNSampler
    {
    public:
        NNDiligentSampler(::Diligent::ISampler* sampler, const NNSamplerDesc& desc);
        ~NNDiligentSampler() override;

        // INNSampler
        const NNSamplerDesc& GetDesc() const override;

        // INNObject
        uint32_t AddRef() override;
        uint32_t Release() override;
        uint32_t GetRefCount() const override;

        // Internal
        ::Diligent::ISampler* GetDiligentSampler() const { return m_Sampler; }

    private:
        ::Diligent::ISampler*   m_Sampler = nullptr;
        NNSamplerDesc           m_Desc;
        std::atomic<uint32_t>   m_RefCount{0};
    };

} // namespace NNDiligent
