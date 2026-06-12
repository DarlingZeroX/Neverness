#pragma once

#include "RmlDiligentRenderTargetPool.h"

#include <RmlUi/Core/Types.h>

#include <vector>

namespace RmlDiligent {

/// RmlUi 层栈：多层 color RT + 共享 DSV + PostPrimary/Secondary/Tertiary + BlendMask。
/// 与 GL3 Backend 一致：PushLayer 递增层索引，CompositeLayers 走 postprocess RT 链。
class RenderLayerStack {
public:
    void Initialize(RenderTargetPool* pool);
    /// 窗口尺寸变化时释放所有层 RT 并通知 Pool 清空空闲列表。
    void OnResize(int width, int height);

    void BeginFrame(int width, int height);
    void EndFrame();

    void SetMsaaSamples(int samples) { m_MsaaSamples = samples; }
    int GetMsaaSamples() const { return m_MsaaSamples; }

    Rml::LayerHandle PushLayer();
    void PopLayer();

    int GetLayerCount() const { return m_LayerCount; }
    Rml::LayerHandle GetTopLayerHandle() const;
    const PooledRenderTarget& GetLayer(Rml::LayerHandle layer) const;
    const PooledRenderTarget& GetTopLayer() const;

    /// 滤镜 ping-pong 主 RT；与 Secondary 交替 Swap。
    PooledRenderTarget& GetPostprocessPrimary();
    PooledRenderTarget& GetPostprocessSecondary();
    PooledRenderTarget& GetPostprocessTertiary();
    PooledRenderTarget& GetBlendMask();
    /// 滤镜 pass 完成后交换 Primary/Secondary，避免额外 CopyTexture。
    void SwapPostprocessPrimarySecondary();

    const PooledRenderTarget* GetSharedDepth() const { return m_SharedDepth.get(); }
    Diligent::ITextureView* GetSharedDepthDSV() const;

private:
    PooledRenderTarget& EnsurePostprocess(int index);

    RenderTargetPool* m_Pool = nullptr;
    int m_Width = 0;
    int m_Height = 0;
    int m_LayerCount = 0;
    int m_MsaaSamples = 1;

    std::vector<RenderTargetPool::PooledRTHandle> m_LayerColors;
    RenderTargetPool::PooledRTHandle m_SharedDepth;

    RenderTargetPool::PooledRTHandle m_PostPrimary;
    RenderTargetPool::PooledRTHandle m_PostSecondary;
    RenderTargetPool::PooledRTHandle m_PostTertiary;
    RenderTargetPool::PooledRTHandle m_BlendMask;
    bool m_PostPrimaryIsIndex0 = true;
};

} // namespace RmlDiligent
