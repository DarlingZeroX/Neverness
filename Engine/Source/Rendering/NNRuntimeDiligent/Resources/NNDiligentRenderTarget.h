#pragma once

#include "../NNDiligentConfig.h"
#include <NNRuntimeRender/RenderTarget/INNRenderTarget.h>
#include <NNRuntimeCore/NNObject.h>
#include <atomic>
#include <vector>

namespace NNDiligent
{
    using namespace NN::Runtime::Core;
    using namespace NN::Runtime::Render;

    class NNDiligentRenderTarget : public INNRenderTarget
    {
    public:
        NNDiligentRenderTarget();
        ~NNDiligentRenderTarget() override;

        // Initialize with pre-created views
        bool Initialize(::Diligent::IRenderDevice* device, const NNRenderTargetDesc& desc);

        // INNRenderTarget
        const NNRenderTargetDesc& GetDesc() const override;
        uint32_t GetWidth() const override;
        uint32_t GetHeight() const override;

        // INNObject
        uint32_t AddRef() override;
        uint32_t Release() override;
        uint32_t GetRefCount() const override;

        // Internal accessors
        ::Diligent::ITextureView* GetColorView(uint32_t index = 0) const;
        ::Diligent::ITextureView* GetColorSRV(uint32_t index = 0) const;  // SRV（用于 ImGui 采样）
        ::Diligent::ITextureView* GetDepthView() const;
        uint32_t GetColorAttachmentCount() const { return static_cast<uint32_t>(m_ColorViews.size()); }

    private:
        NNRenderTargetDesc m_Desc;

        // Owned textures (the RT owns the textures and views)
        std::vector<::Diligent::ITexture*>      m_ColorTextures;
        ::Diligent::ITexture*                   m_DepthTexture = nullptr;

        // Views (non-owning, released with texture)
        std::vector<::Diligent::ITextureView*>  m_ColorViews;   // RTV
        std::vector<::Diligent::ITextureView*>  m_ColorSRVs;    // SRV
        ::Diligent::ITextureView*               m_DepthView = nullptr;

        std::atomic<uint32_t> m_RefCount{0};
    };

} // namespace NNDiligent
