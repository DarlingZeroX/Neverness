// NNDiligentTexture.cpp — Diligent Texture wrapper

#include "../../Resources/NNDiligentTexture.h"

namespace NNDiligent
{
    NNDiligentTexture::NNDiligentTexture(::Diligent::ITexture* texture, const NNTextureDesc& desc)
        : m_Texture(texture)
        , m_Desc(desc)
    {
        if (m_Texture) m_Texture->AddRef();
    }

    NNDiligentTexture::~NNDiligentTexture()
    {
        if (m_Texture)
        {
            m_Texture->Release();
            m_Texture = nullptr;
        }
    }

    const NNTextureDesc& NNDiligentTexture::GetDesc() const
    {
        return m_Desc;
    }

    uint32_t NNDiligentTexture::GetWidth() const
    {
        return m_Desc.Width;
    }

    uint32_t NNDiligentTexture::GetHeight() const
    {
        return m_Desc.Height;
    }

    ::Diligent::ITextureView* NNDiligentTexture::GetDefaultView(::Diligent::TEXTURE_VIEW_TYPE viewType)
    {
        if (!m_Texture) return nullptr;

        switch (viewType)
        {
            case ::Diligent::TEXTURE_VIEW_SHADER_RESOURCE:
                return m_Texture->GetDefaultView(::Diligent::TEXTURE_VIEW_SHADER_RESOURCE);
            case ::Diligent::TEXTURE_VIEW_RENDER_TARGET:
                return m_Texture->GetDefaultView(::Diligent::TEXTURE_VIEW_RENDER_TARGET);
            case ::Diligent::TEXTURE_VIEW_DEPTH_STENCIL:
                return m_Texture->GetDefaultView(::Diligent::TEXTURE_VIEW_DEPTH_STENCIL);
            case ::Diligent::TEXTURE_VIEW_UNORDERED_ACCESS:
                return m_Texture->GetDefaultView(::Diligent::TEXTURE_VIEW_UNORDERED_ACCESS);
            default:
                return nullptr;
        }
    }

    uint32_t NNDiligentTexture::AddRef()
    {
        return ++m_RefCount;
    }

    uint32_t NNDiligentTexture::Release()
    {
        uint32_t c = --m_RefCount;
        if (c == 0)
            delete this;
        return c;
    }

    uint32_t NNDiligentTexture::GetRefCount() const
    {
        return m_RefCount;
    }

} // namespace NNDiligent
