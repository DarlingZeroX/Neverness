// =============================================================================
// RmlDiligentRenderInterface.h
// RmlUi 的 Diligent Engine 渲染接口实现
// =============================================================================

#pragma once
#ifndef RML_PERF_COUNTERS
#define RML_PERF_COUNTERS
#endif
#include <RmlUi/Core/Matrix4.h>
#include <RmlUi/Core/RenderInterface.h>
#include <RmlUi/Core/Types.h>

#include "RmlDiligentBufferManager.h"  // BufferAllocation 结构体（GeometryHandle 使用）
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
    BufferAllocation vbAlloc;  // BufferManager 子分配（共享大 buffer + offset）
    BufferAllocation ibAlloc;
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
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_SRB_Color_StencilEqual;  // 为 m_PSO_Color_StencilEqual 创建

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

    // RenderStateCache：PSO/SRB 脏检测，过滤冗余驱动调用
    struct RenderStateCache {
        Diligent::IPipelineState* currentPSO = nullptr;
        Diligent::IShaderResourceBinding* currentSRB = nullptr;
        void Reset() { currentPSO = nullptr; currentSRB = nullptr; }
    } m_StateCache;

    // 绑定 PSO（脏检测：指针比较）。PSO 变化时自动清空 SRB 缓存。
    void BindPSO(Diligent::IPipelineState* pso);
    // 绑定 SRB（脏检测：指针比较）。mode 保留原有值，缓存比较只看 srb 指针。
    void BindSRB(Diligent::IShaderResourceBinding* srb,
                 Diligent::RESOURCE_STATE_TRANSITION_MODE mode = Diligent::RESOURCE_STATE_TRANSITION_MODE_NONE);

#ifdef RML_PERF_COUNTERS
    // 性能计数器：累计值 + 上一次快照（用于计算 delta）
    struct PerfCounters {
        // 累计值
        uint32_t compileGeometry = 0;     // CompileGeometry 调用次数
        uint32_t releaseGeometry = 0;     // ReleaseGeometry 调用次数
        uint32_t createBuffer = 0;        // CreateBuffer 调用次数
        uint32_t renderGeometry = 0;      // RenderGeometry 调用次数
        uint32_t renderShader = 0;        // RenderShader 调用次数
        uint32_t setPSOReq = 0;           // BindPSO 请求次数
        uint32_t setPSOReal = 0;          // 实际 SetPipelineState 次数
        uint32_t commitSRBReq = 0;        // BindSRB 请求次数
        uint32_t commitSRBReal = 0;       // 实际 CommitShaderResources 次数
        uint32_t drawIndexed = 0;         // DrawIndexed 调用次数
        uint32_t drawFullscreen = 0;      // DrawFullscreenPassthrough 调用次数
        uint32_t mapCB = 0;              // ConstantBuffer Map/Unmap 次数

        // 上一次快照（用于计算每帧增量）
        uint32_t prevCompileGeometry = 0;
        uint32_t prevReleaseGeometry = 0;
        uint32_t prevCreateBuffer = 0;
        uint32_t prevRenderGeometry = 0;
        int64_t prevCompileGeometryUs = 0;  // 用于计算每帧 CompileGeometry 耗时增量

        // 帧计时（微秒）
        int64_t beginFrameUs = 0;         // BeginFrame 耗时
        int64_t endFrameUs = 0;           // EndFrame 耗时（不含 Present）
        int64_t presentUs = 0;            // Present 耗时
        int64_t frameTotalUs = 0;         // 整帧耗时（BeginFrame + 所有 draw + EndFrame + Present）

        // 每类操作的累计耗时（微秒），用于定位热点
        int64_t compileGeometryUs = 0;    // CompileGeometry 总耗时（含 CreateBuffer）
        int64_t releaseGeometryUs = 0;    // ReleaseGeometry 总耗时
        int64_t bindPSOUs = 0;            // BindPSO 总耗时（含 SetPipelineState）
        int64_t bindSRBUs = 0;            // BindSRB 总耗时（含 CommitShaderResources）
        int64_t mapCBUs = 0;              // Map/Unmap CB 总耗时
        int64_t drawIndexedUs = 0;        // DrawIndexed 总耗时
        int64_t setScissorUs = 0;         // SetScissorRects 总耗时
        int64_t setVBIBUs = 0;            // SetVertexBuffers + SetIndexBuffer 总耗时
        int64_t setStencilRefUs = 0;      // SetStencilRef 总耗时
        int64_t srbLookupUs = 0;          // GetOrCreateTextureSRB 总耗时

        // 帧计时用的时间戳
        int64_t frameStartTick = 0;       // BeginFrame 入口
        int64_t beginFrameEndTick = 0;    // BeginFrame 出口
        int64_t endFrameStartTick = 0;    // EndFrame 入口
        int64_t presentStartTick = 0;     // Present 前
        int64_t presentEndTick = 0;       // Present 后
        int64_t updateStartTick = 0;      // context->Update() 前
        int64_t updateEndTick = 0;        // context->Update() 后
        int64_t renderStartTick = 0;      // context->Render() 前
        int64_t renderEndTick = 0;        // context->Render() 后
        int64_t flushStartTick = 0;       // Flush 前
        int64_t flushEndTick = 0;         // Flush 后
        int64_t idleGPUStartTick = 0;     // IdleGPU 前
        int64_t idleGPUEndTick = 0;       // IdleGPU 后

        // RmlUi CPU 侧耗时
        int64_t rmlUpdateUs = 0;
        int64_t rmlRenderUs = 0;

        // GPU 同步耗时
        int64_t flushUs = 0;
        int64_t idleGPUUs = 0;
    } m_PerfCounters;

    // 应用层在各阶段前后调用，用于定位 CPU 瓶颈
    void MarkPresentStart();
    void MarkPresentEnd();
    void MarkEndFrameStart();
    void MarkUpdateStart();       // context->Update() 前
    void MarkUpdateEnd();         // context->Update() 后
    void MarkRenderStart();       // context->Render() 前
    void MarkRenderEnd();         // context->Render() 后
    void MarkFlushStart();        // m_Context->Flush() 前
    void MarkFlushEnd();          // m_Context->Flush() 后
    void MarkIdleGPUStart();      // m_Device->IdleGPU() 前
    void MarkIdleGPUEnd();        // m_Device->IdleGPU() 后
#endif
};

} // namespace RmlDiligent
