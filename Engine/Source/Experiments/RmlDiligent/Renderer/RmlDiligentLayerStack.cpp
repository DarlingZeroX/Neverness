#include "RmlDiligentLayerStack.h"

#include <cassert>

namespace RmlDiligent {

// =============================================================================
// RenderLayerStack — RmlUi 离屏层与 postprocess RT 管理
// =============================================================================

void RenderLayerStack::Initialize(RenderTargetPool* pool)
{
    m_Pool = pool;
}

void RenderLayerStack::OnResize(int width, int height)
{
    if (width == m_Width && height == m_Height) {
        return;
    }

    m_Width = width;
    m_Height = height;
    m_LayerCount = 0;
    m_LayerColors.clear();
    m_SharedDepth.reset();
    m_PostPrimary.reset();
    m_PostSecondary.reset();
    m_PostTertiary.reset();
    m_BlendMask.reset();
    m_PostPrimaryIsIndex0 = true;

    if (m_Pool) {
        m_Pool->OnResize(width, height);
    }
}

void RenderLayerStack::BeginFrame(int width, int height)
{
    assert(m_LayerCount == 0);

    if (width != m_Width || height != m_Height) {
        OnResize(width, height);
    }

    PushLayer();
}

void RenderLayerStack::EndFrame()
{
    assert(m_LayerCount == 1);
    PopLayer();
    assert(m_LayerCount == 0);
}

Rml::LayerHandle RenderLayerStack::PushLayer()
{
    if (!m_Pool || m_Width <= 0 || m_Height <= 0) {
        return {};
    }

    if (m_LayerCount == static_cast<int>(m_LayerColors.size())) {
        if (m_MsaaSamples > 1) {
            m_LayerColors.push_back(RenderTargetPool::AcquireColorMSAA(*m_Pool, m_Width, m_Height, m_MsaaSamples));
        } else {
            m_LayerColors.push_back(RenderTargetPool::AcquireColor(*m_Pool, m_Width, m_Height));
        }
    }

    if (!m_SharedDepth) {
        if (m_MsaaSamples > 1) {
            m_SharedDepth = RenderTargetPool::AcquireDepthStencilMSAA(*m_Pool, m_Width, m_Height, m_MsaaSamples);
        } else {
            m_SharedDepth = RenderTargetPool::AcquireDepthStencil(*m_Pool, m_Width, m_Height);
        }
    }

    ++m_LayerCount;
    return GetTopLayerHandle();
}

void RenderLayerStack::PopLayer()
{
    if (m_LayerCount > 0) {
        --m_LayerCount;
    }
}

Rml::LayerHandle RenderLayerStack::GetTopLayerHandle() const
{
    if (m_LayerCount <= 0) {
        return {};
    }
    return static_cast<Rml::LayerHandle>(m_LayerCount - 1);
}

const PooledRenderTarget& RenderLayerStack::GetLayer(Rml::LayerHandle layer) const
{
    static PooledRenderTarget empty{};
    const int index = static_cast<int>(layer);
    if (index < 0 || index >= m_LayerCount || index >= static_cast<int>(m_LayerColors.size())) {
        return empty;
    }
    const auto& handle = m_LayerColors[static_cast<size_t>(index)];
    return handle ? *handle : empty;
}

const PooledRenderTarget& RenderLayerStack::GetTopLayer() const
{
    return GetLayer(GetTopLayerHandle());
}

PooledRenderTarget& RenderLayerStack::EnsurePostprocess(int index)
{
    static PooledRenderTarget empty{};
    if (!m_Pool || m_Width <= 0 || m_Height <= 0) {
        return empty;
    }

    if (index == 0) {
        if (!m_PostPrimary) {
            m_PostPrimary = RenderTargetPool::AcquirePostprocess(*m_Pool, m_Width, m_Height);
        }
        return m_PostPrimary ? *m_PostPrimary : empty;
    }

    if (index == 1) {
        if (!m_PostSecondary) {
            m_PostSecondary = RenderTargetPool::AcquirePostprocess(*m_Pool, m_Width, m_Height);
        }
        return m_PostSecondary ? *m_PostSecondary : empty;
    }

    if (index == 2) {
        if (!m_PostTertiary) {
            m_PostTertiary = RenderTargetPool::AcquirePostprocess(*m_Pool, m_Width, m_Height);
        }
        return m_PostTertiary ? *m_PostTertiary : empty;
    }

    if (!m_BlendMask) {
        m_BlendMask = RenderTargetPool::AcquirePostprocess(*m_Pool, m_Width, m_Height);
    }
    return m_BlendMask ? *m_BlendMask : empty;
}

PooledRenderTarget& RenderLayerStack::GetPostprocessPrimary()
{
    return m_PostPrimaryIsIndex0 ? EnsurePostprocess(0) : EnsurePostprocess(1);
}

PooledRenderTarget& RenderLayerStack::GetPostprocessSecondary()
{
    return m_PostPrimaryIsIndex0 ? EnsurePostprocess(1) : EnsurePostprocess(0);
}

PooledRenderTarget& RenderLayerStack::GetPostprocessTertiary()
{
    return EnsurePostprocess(2);
}

PooledRenderTarget& RenderLayerStack::GetBlendMask()
{
    return EnsurePostprocess(3);
}

void RenderLayerStack::SwapPostprocessPrimarySecondary()
{
    m_PostPrimaryIsIndex0 = !m_PostPrimaryIsIndex0;
}

Diligent::ITextureView* RenderLayerStack::GetSharedDepthDSV() const
{
    if (!m_SharedDepth || !m_SharedDepth->DSV) {
        return nullptr;
    }
    return m_SharedDepth->DSV;
}

} // namespace RmlDiligent
