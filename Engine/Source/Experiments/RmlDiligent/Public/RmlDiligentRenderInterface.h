// =============================================================================
// RmlDiligentRenderInterface.h
// RmlUi 的 Diligent Engine 渲染接口实现
// =============================================================================

#pragma once

#include <RmlUi/Core/Matrix4.h>
#include <RmlUi/Core/RenderInterface.h>
#include <RmlUi/Core/Types.h>

#include "RmlDiligentLayerStack.h"
#include "RmlDiligentProgramId.h"
#include "RmlDiligentRenderTargetPool.h"
#include "RmlDiligentSrbCache.h"

#include "Common/interface/RefCntAutoPtr.hpp"
#include "Graphics/GraphicsEngine/interface/ShaderResourceBinding.h"
#include "Graphics/GraphicsEngine/interface/RenderDevice.h"
#include "Graphics/GraphicsEngine/interface/DeviceContext.h"
#include "Graphics/GraphicsEngine/interface/SwapChain.h"
#include "Graphics/GraphicsEngine/interface/PipelineState.h"
#include "Graphics/GraphicsEngine/interface/Buffer.h"
#include "Graphics/GraphicsEngine/interface/Texture.h"
#include "Graphics/GraphicsEngine/interface/TextureView.h"
#include "Graphics/GraphicsEngine/interface/Sampler.h"
#include "Graphics/GraphicsEngine/interface/RenderPass.h"

#include <cstddef>
#include <unordered_map>

namespace RmlDiligent {

/// Phase 6 内存与缓存占用快照（只读，供 TestPerformance / 调试）。
struct MemoryStats {
    size_t pooledRtFreeCount = 0;
    size_t pooledRtActiveLayers = 0;
    size_t srbCacheEntries = 0;
    size_t textureSrbEntries = 0;
};

struct GeometryHandle {
    Diligent::RefCntAutoPtr<Diligent::IBuffer> vertexBuffer;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> indexBuffer;
    uint32_t vertexCount;
    uint32_t indexCount;
};

struct TextureHandle {
    Diligent::RefCntAutoPtr<Diligent::ITexture> texture;
    Diligent::RefCntAutoPtr<Diligent::ITextureView> SRV;
    RenderTargetPool::PooledRTHandle pooled;
    bool fromPool = false;
    /// 按 ProgramId 懒创建；同一纹理可绑定 Texture/Blur 等不同 PSO。
    std::unordered_map<ProgramId, Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding>> srbCache;
};

class RmlDiligentRenderInterface : public Rml::RenderInterface {
public:
    RmlDiligentRenderInterface();
    ~RmlDiligentRenderInterface();

    bool Initialize(Diligent::IRenderDevice* device,
                    Diligent::IDeviceContext* context,
                    Diligent::ISwapChain* swapChain);

    void SetProjectionMatrix(int width, int height);

    void BeginFrame();
    void EndFrame();

    void DrawDebugQuad();

    uint32_t GetDrawCount() const { return m_DrawCount; }
    void ResetDrawCount()
    {
        m_DrawCount = 0;
        m_TextureDrawCount = 0;
        m_ShaderDrawCount = 0;
        m_ClipMaskDrawCount = 0;
        m_CallbackTextureDrawCount = 0;
    }
    uint32_t GetTextureDrawCount() const { return m_TextureDrawCount; }
    uint32_t GetCallbackTextureDrawCount() const { return m_CallbackTextureDrawCount; }
    uint32_t GetSaveLayerAsTextureCount() const { return m_SaveLayerAsTextureCount; }
    uint32_t GetShaderDrawCount() const { return m_ShaderDrawCount; }
    uint32_t GetCompileShaderCount() const { return m_CompileShaderCount; }
    uint32_t GetClipMaskDrawCount() const { return m_ClipMaskDrawCount; }
    uint32_t GetPushLayerCount() const { return m_PushLayerCount; }
    uint32_t GetCompositeCount() const { return m_CompositeCount; }
    uint32_t GetFilterRenderCount() const { return m_FilterRenderCount; }

    MemoryStats GetMemoryStats() const;
    void SetSrbCacheEnabled(bool enabled) { m_SrbCacheEnabled = enabled; }
    bool IsSrbCacheEnabled() const { return m_SrbCacheEnabled; }
    void ResetPerfStats();
    uint32_t GetSrbCacheHits() const { return m_SrbCache.GetHits(); }
    uint32_t GetSrbCacheMisses() const { return m_SrbCache.GetMisses(); }
    size_t GetSrbCacheEntryCount() const { return m_SrbCache.GetEntryCount(); }
    size_t GetTextureSrbEntryCount() const { return m_TextureSrbEntryCount; }

    Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex> vertices,
                                                 Rml::Span<const int> indices) override;

    void RenderGeometry(Rml::CompiledGeometryHandle geometry,
                        Rml::Vector2f translation,
                        Rml::TextureHandle texture) override;

    void ReleaseGeometry(Rml::CompiledGeometryHandle geometry) override;

    Rml::TextureHandle LoadTexture(Rml::Vector2i& texture_dimensions,
                                    const Rml::String& source) override;

    Rml::TextureHandle GenerateTexture(Rml::Span<const Rml::byte> source,
                                        Rml::Vector2i source_dimensions) override;

    void ReleaseTexture(Rml::TextureHandle texture) override;

    void EnableScissorRegion(bool enable) override;
    void SetScissorRegion(Rml::Rectanglei region) override;

    void SetTransform(const Rml::Matrix4f* transform) override;

    void EnableClipMask(bool enable) override;
    void RenderToClipMask(Rml::ClipMaskOperation operation, Rml::CompiledGeometryHandle geometry,
                          Rml::Vector2f translation) override;

    Rml::CompiledShaderHandle CompileShader(const Rml::String& name, const Rml::Dictionary& parameters) override;
    void RenderShader(Rml::CompiledShaderHandle shader, Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation,
                      Rml::TextureHandle texture) override;
    void ReleaseShader(Rml::CompiledShaderHandle shader) override;

    Rml::LayerHandle PushLayer() override;
    void CompositeLayers(Rml::LayerHandle source, Rml::LayerHandle destination, Rml::BlendMode blend_mode,
                         Rml::Span<const Rml::CompiledFilterHandle> filters) override;
    void PopLayer() override;

    Rml::TextureHandle SaveLayerAsTexture() override;

    Rml::CompiledFilterHandle CompileFilter(const Rml::String& name, const Rml::Dictionary& parameters) override;
    void ReleaseFilter(Rml::CompiledFilterHandle filter) override;
    Rml::CompiledFilterHandle SaveLayerAsMaskImage() override;

private:
    void BindLayer(Rml::LayerHandle layer);
    void BindTopLayer();
    void UnbindRenderTargets();
    void BlitLayerToPostprocessPrimary(Rml::LayerHandle layer_handle);
    void DrawFullscreenPassthrough(Diligent::ITextureView* sourceSRV, Diligent::ITextureView* destRTV,
                                   Rml::BlendMode blend_mode, bool use_layer_depth_stencil = true,
                                   Diligent::IPipelineState* pso_override = nullptr,
                                   bool reset_default_uv = true);
    void DrawFullscreenPassthroughUV(Diligent::ITextureView* sourceSRV, Diligent::ITextureView* destRTV,
                                     Rml::Vector2f uv_offset, Rml::Vector2f uv_scaling,
                                     Diligent::IPipelineState* pso_override = nullptr);
    void RenderFilters(Rml::Span<const Rml::CompiledFilterHandle> filters);
    void RenderBlur(float sigma, PooledRenderTarget& source_destination, PooledRenderTarget& temp,
                    const Rml::Rectanglei& blur_region);
    void UploadBlurCB(float sigma, const Rml::Vector2f& texel_offset, const Rml::Rectanglei& scissor,
                      const Rml::Vector2i& framebuffer_size);
    void UploadDropShadowCB(const Rml::Colourf& color, const Rml::Rectanglei& scissor, const Rml::Vector2i& framebuffer_size);
    void UploadColorMatrixCB(const Rml::Matrix4f& color_matrix);
    void SetSwapchainViewport();
    void SetFramebufferViewport(int width, int height);
    void SetScissorRml(const Rml::Rectanglei& region, bool vertically_flip = false);
    void BlitFramebuffer(Diligent::ITextureView* sourceSRV, Diligent::ITextureView* destRTV, int fb_width, int fb_height,
                         int srcX0, int srcY0, int srcX1, int srcY1, int dstX0, int dstY0, int dstX1, int dstY1);
    void DrawBlurPass(Diligent::ITextureView* sourceSRV, Diligent::ITextureView* destRTV,
                      const Rml::Rectanglei& scissor_rect, int fb_width, int fb_height);
    void CreateRenderPass(int msaa_samples = 1);
    void CreatePSOs();
    void CreateConstantBuffer();
    void CreateShaderConstantBuffers();
    void DrawIndexedGeometry(GeometryHandle* geom);
    void DrawColorGeometry(GeometryHandle* geom, Rml::Vector2f translation, Diligent::IPipelineState* pso);
    void EnsureFramebufferBound();
    Diligent::ITextureView* GetActiveDepthStencilDSV() const;
    Diligent::IShaderResourceBinding* GetOrCreateTextureSRB(TextureHandle* tex, ProgramId id, Diligent::IPipelineState* pso);
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> GetOrCreateTextureSRBRef(
        TextureHandle* tex, ProgramId id, Diligent::IPipelineState* pso);
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> GetOrCreateCachedSRB(
        Diligent::IPipelineState* pso,
        Diligent::ITextureView* primarySrv,
        Diligent::IBuffer* constantBuffer,
        const SrbCache::BindFn& bindFn,
        Diligent::ITextureView* secondarySrv = nullptr);
    void ClearSrbCachesOnResize(int width, int height);

    Diligent::IRenderDevice* m_Device = nullptr;
    Diligent::IDeviceContext* m_Context = nullptr;
    Diligent::ISwapChain* m_SwapChain = nullptr;

    RenderTargetPool m_RTPool;
    RenderLayerStack m_LayerStack;

    Diligent::RefCntAutoPtr<Diligent::IRenderPass> m_RenderPass;

    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_PSO_Color;
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_PSO_Texture;
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_PSO_Gradient;
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_PSO_Creation;

    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_PSO_Color_StencilEqual;
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_PSO_Texture_StencilEqual;
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_PSO_Gradient_StencilEqual;
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_PSO_Creation_StencilEqual;
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_PSO_Color_StencilSet;
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_PSO_Color_StencilIntersect;

    Diligent::RefCntAutoPtr<Diligent::IShader> m_VS_Color;
    Diligent::RefCntAutoPtr<Diligent::IShader> m_VS_Texture;
    Diligent::RefCntAutoPtr<Diligent::IShader> m_PS_Color;
    Diligent::RefCntAutoPtr<Diligent::IShader> m_PS_Texture;
    Diligent::RefCntAutoPtr<Diligent::IShader> m_PS_Gradient;
    Diligent::RefCntAutoPtr<Diligent::IShader> m_PS_Creation;

    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_ConstantBuffer;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_ProjectionCB; // 全局投影矩阵 CB
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_GradientCB;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_CreationCB;

    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_SRB_Color;

    Diligent::RefCntAutoPtr<Diligent::ISampler> m_Sampler;

    Diligent::RefCntAutoPtr<Diligent::IShader> m_VS_PassThrough;
    Diligent::RefCntAutoPtr<Diligent::IShader> m_PS_Passthrough;
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_PSO_Passthrough;
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_PSO_Passthrough_StencilEqual;
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_PSO_PassthroughPresent;
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_PSO_PassthroughOpacity;
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_PSO_PassthroughReplace;
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_PSO_Blur;
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_PSO_DropShadow;
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_PSO_ColorMatrix;
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_PSO_BlendMask;
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_PSO_Composite;
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_PSO_CompositeReplace;
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_PSO_Composite_StencilEqual;  // MSAA + StencilEqual

    Diligent::RefCntAutoPtr<Diligent::IShader> m_VS_Blur;
    Diligent::RefCntAutoPtr<Diligent::IShader> m_PS_Blur;
    Diligent::RefCntAutoPtr<Diligent::IShader> m_PS_DropShadow;
    Diligent::RefCntAutoPtr<Diligent::IShader> m_PS_ColorMatrix;
    Diligent::RefCntAutoPtr<Diligent::IShader> m_PS_BlendMask;

    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_BlurCB;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_DropShadowCB;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_ColorMatrixCB;

    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_FullscreenVB;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_FullscreenIB;

    bool m_ScissorEnabled = false;
    Diligent::Rect m_ScissorRect{0, 0, 0, 0};
    Rml::Rectanglei m_ScissorRegionRml;

    Rml::Matrix4f m_Projection;
    Rml::Matrix4f m_Transform; // CSS transform（不含投影）

    bool m_SwapchainPassActive = false;

    uint32_t m_DrawCount = 0;
    uint32_t m_TextureDrawCount = 0;
    uint32_t m_ShaderDrawCount = 0;
    uint32_t m_CompileShaderCount = 0;
    uint32_t m_ClipMaskDrawCount = 0;
    uint32_t m_CallbackTextureDrawCount = 0;
    uint32_t m_SaveLayerAsTextureCount = 0;
    uint32_t m_PushLayerCount = 0;

    bool m_ClipMaskEnabled = false;
    bool m_UseStencilEqual = false;
    uint8_t m_StencilTestValue = 0;
    uint32_t m_CompositeCount = 0;
    uint32_t m_FilterRenderCount = 0;
    bool m_CBWarnPrinted = false;

    SrbCache m_SrbCache;
    bool m_SrbCacheEnabled = true;
    size_t m_TextureSrbEntryCount = 0;
    int m_CachedWidth = 0;
    int m_CachedHeight = 0;
    int m_MsaaSamples = 1;
};

} // namespace RmlDiligent
