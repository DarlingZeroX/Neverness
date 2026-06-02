// NNDiligentRenderTarget.cpp — Diligent RenderTarget wrapper

#include "../../Resources/NNDiligentRenderTarget.h"
#include <NNRuntimeRender/Resources/INNTexture.h>

namespace NNDiligent
{
    // Helper: Convert NNEngine PixelFormat to Diligent TEXTURE_FORMAT
    static ::Diligent::TEXTURE_FORMAT ToDiligentFormat(NNPixelFormat fmt)
    {
        switch (fmt)
        {
            case NNPixelFormat::RGBA8_UNORM:     return ::Diligent::TEX_FORMAT_RGBA8_UNORM;
            case NNPixelFormat::RGBA8_SRGB:      return ::Diligent::TEX_FORMAT_RGBA8_UNORM_SRGB;
            case NNPixelFormat::BGRA8_UNORM:     return ::Diligent::TEX_FORMAT_BGRA8_UNORM;
            case NNPixelFormat::R32_FLOAT:       return ::Diligent::TEX_FORMAT_R32_FLOAT;
            case NNPixelFormat::RG32_FLOAT:      return ::Diligent::TEX_FORMAT_RG32_FLOAT;
            case NNPixelFormat::RGBA32_FLOAT:    return ::Diligent::TEX_FORMAT_RGBA32_FLOAT;
            case NNPixelFormat::D32_FLOAT:       return ::Diligent::TEX_FORMAT_D32_FLOAT;
            case NNPixelFormat::D24_UNORM_S8_UINT: return ::Diligent::TEX_FORMAT_D24_UNORM_S8_UINT;
            case NNPixelFormat::BC1_UNORM:       return ::Diligent::TEX_FORMAT_BC1_UNORM;
            case NNPixelFormat::BC3_UNORM:       return ::Diligent::TEX_FORMAT_BC3_UNORM;
            case NNPixelFormat::BC5_UNORM:       return ::Diligent::TEX_FORMAT_BC5_UNORM;
            case NNPixelFormat::BC7_UNORM:       return ::Diligent::TEX_FORMAT_BC7_UNORM;
            default:                             return ::Diligent::TEX_FORMAT_UNKNOWN;
        }
    }

    NNDiligentRenderTarget::NNDiligentRenderTarget() {}

    NNDiligentRenderTarget::~NNDiligentRenderTarget()
    {
        // Release views first (they are non-owning, but clear the vector)
        m_ColorViews.clear();
        m_DepthView = nullptr;

        // Release owned textures
        for (auto* tex : m_ColorTextures)
        {
            if (tex) tex->Release();
        }
        m_ColorTextures.clear();

        if (m_DepthTexture)
        {
            m_DepthTexture->Release();
            m_DepthTexture = nullptr;
        }
    }

    bool NNDiligentRenderTarget::Initialize(::Diligent::IRenderDevice* device, const NNRenderTargetDesc& desc)
    {
        if (!device) return false;

        m_Desc = desc;

        // Create color attachments
        for (uint32_t i = 0; i < desc.ColorAttachmentCount; ++i)
        {
            ::Diligent::TextureDesc texDesc;
            texDesc.Name = desc.DebugName ? desc.DebugName : "RT_Color";
            texDesc.Type = ::Diligent::RESOURCE_DIM_TEX_2D;
            texDesc.Width = desc.Width;
            texDesc.Height = desc.Height;
            texDesc.Format = ToDiligentFormat(desc.ColorFormat);
            texDesc.MipLevels = 1;
            texDesc.SampleCount = desc.SampleCount;
            texDesc.BindFlags = ::Diligent::BIND_RENDER_TARGET | ::Diligent::BIND_SHADER_RESOURCE;

            ::Diligent::ITexture* tex = nullptr;
            device->CreateTexture(texDesc, nullptr, &tex);
            if (!tex)
            {
                // Cleanup already created textures
                for (auto* t : m_ColorTextures) if (t) t->Release();
                m_ColorTextures.clear();
                return false;
            }

            m_ColorTextures.push_back(tex);
            m_ColorViews.push_back(tex->GetDefaultView(::Diligent::TEXTURE_VIEW_RENDER_TARGET));
        }

        // Create depth attachment (if depth format is specified)
        if (desc.DepthFormat != NNPixelFormat::Unknown)
        {
            ::Diligent::TextureDesc texDesc;
            texDesc.Name = desc.DebugName ? desc.DebugName : "RT_Depth";
            texDesc.Type = ::Diligent::RESOURCE_DIM_TEX_2D;
            texDesc.Width = desc.Width;
            texDesc.Height = desc.Height;
            texDesc.Format = ToDiligentFormat(desc.DepthFormat);
            texDesc.MipLevels = 1;
            texDesc.SampleCount = desc.SampleCount;
            texDesc.BindFlags = ::Diligent::BIND_DEPTH_STENCIL;

            device->CreateTexture(texDesc, nullptr, &m_DepthTexture);
            if (m_DepthTexture)
            {
                m_DepthView = m_DepthTexture->GetDefaultView(::Diligent::TEXTURE_VIEW_DEPTH_STENCIL);
            }
        }

        return true;
    }

    const NNRenderTargetDesc& NNDiligentRenderTarget::GetDesc() const
    {
        return m_Desc;
    }

    uint32_t NNDiligentRenderTarget::GetWidth() const
    {
        return m_Desc.Width;
    }

    uint32_t NNDiligentRenderTarget::GetHeight() const
    {
        return m_Desc.Height;
    }

    ::Diligent::ITextureView* NNDiligentRenderTarget::GetColorView(uint32_t index) const
    {
        if (index >= m_ColorViews.size()) return nullptr;
        return m_ColorViews[index];
    }

    ::Diligent::ITextureView* NNDiligentRenderTarget::GetDepthView() const
    {
        return m_DepthView;
    }

    uint32_t NNDiligentRenderTarget::AddRef()
    {
        return ++m_RefCount;
    }

    uint32_t NNDiligentRenderTarget::Release()
    {
        uint32_t c = --m_RefCount;
        if (c == 0)
            delete this;
        return c;
    }

    uint32_t NNDiligentRenderTarget::GetRefCount() const
    {
        return m_RefCount;
    }

} // namespace NNDiligent
