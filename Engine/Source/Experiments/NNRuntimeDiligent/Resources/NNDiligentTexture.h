#pragma once

#include "../NNDiligentConfig.h"
#include <NNRuntimeRender/Resources/INNTexture.h>
#include <NNRuntimeCore/NNObject.h>
#include <atomic>

namespace NNDiligent
{
    using namespace NN::Runtime::Core;
    using namespace NN::Runtime::Render;

    class NNDiligentTexture : public INNTexture
    {
    public:
        NNDiligentTexture(::Diligent::ITexture* texture, const NNTextureDesc& desc);
        ~NNDiligentTexture() override;

        // INNTexture
        const NNTextureDesc& GetDesc() const override;
        uint32_t GetWidth() const override;
        uint32_t GetHeight() const override;

        // INNObject
        uint32_t AddRef() override;
        uint32_t Release() override;
        uint32_t GetRefCount() const override;

        // Internal
        ::Diligent::ITexture* GetDiligentTexture() const { return m_Texture; }

        // Views (created on demand)
        ::Diligent::ITextureView* GetDefaultView(::Diligent::TEXTURE_VIEW_TYPE viewType);

    private:
        ::Diligent::ITexture*   m_Texture = nullptr;
        NNTextureDesc           m_Desc;
        std::atomic<uint32_t>   m_RefCount{0};
    };

} // namespace NNDiligent
