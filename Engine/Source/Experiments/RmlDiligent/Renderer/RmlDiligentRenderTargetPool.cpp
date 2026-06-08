#include "RmlDiligentRenderTargetPool.h"

namespace RmlDiligent {

// =============================================================================
// RenderTargetPool — 尺寸匹配的空闲 RT 复用；OnResize 清空列表（SRB 缓存需同步 Clear）
// =============================================================================

void RenderTargetPool::Deleter::operator()(PooledRenderTarget* rt) const
{
    if (!rt) {
        return;
    }
    if (rt->pool) {
        rt->pool->Return(rt);
    } else {
        delete rt;
    }
}

void RenderTargetPool::Initialize(Diligent::IRenderDevice* device)
{
    m_Device = device;
}

void RenderTargetPool::OnResize(int width, int height)
{
    if (width == m_Width && height == m_Height) {
        return;
    }
    m_Width = width;
    m_Height = height;
    m_ActiveAcquireCount = 0;
    m_FreeList.clear();
}

PooledRenderTarget* RenderTargetPool::CreateNew(
    int width,
    int height,
    Diligent::BIND_FLAGS bindFlags,
    Diligent::TEXTURE_FORMAT format,
    int samples)
{
    if (!m_Device || width <= 0 || height <= 0) {
        return nullptr;
    }

    auto* entry = new PooledRenderTarget();
    entry->pool = this;
    entry->width = width;
    entry->height = height;
    entry->samples = samples;
    entry->bindFlags = bindFlags;
    entry->format = format;

    Diligent::TextureDesc texDesc;
    texDesc.Name = "RmlDiligent PooledRT";
    texDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
    texDesc.Width = static_cast<Diligent::Uint32>(width);
    texDesc.Height = static_cast<Diligent::Uint32>(height);
    texDesc.MipLevels = 1;
    texDesc.Format = format;
    texDesc.BindFlags = bindFlags;
    texDesc.SampleCount = static_cast<Diligent::Uint32>(samples);

    m_Device->CreateTexture(texDesc, nullptr, &entry->texture);
    if (!entry->texture) {
        delete entry;
        return nullptr;
    }

    if (bindFlags & Diligent::BIND_RENDER_TARGET) {
        entry->RTV = entry->texture->GetDefaultView(Diligent::TEXTURE_VIEW_RENDER_TARGET);
    }
    if (bindFlags & Diligent::BIND_SHADER_RESOURCE) {
        entry->SRV = entry->texture->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);
    }
    if (bindFlags & Diligent::BIND_DEPTH_STENCIL) {
        entry->DSV = entry->texture->GetDefaultView(Diligent::TEXTURE_VIEW_DEPTH_STENCIL);
    }

    return entry;
}

RenderTargetPool::PooledRTHandle RenderTargetPool::Acquire(
    int width,
    int height,
    Diligent::BIND_FLAGS bindFlags,
    Diligent::TEXTURE_FORMAT format,
    int samples)
{
    for (auto it = m_FreeList.begin(); it != m_FreeList.end(); ++it) {
        auto& candidate = *it;
        if (candidate && candidate->width == width && candidate->height == height && candidate->bindFlags == bindFlags
            && candidate->format == format && candidate->samples == samples) {
            PooledRenderTarget* raw = candidate.release();
            m_FreeList.erase(it);
            ++m_ActiveAcquireCount;
            return PooledRTHandle(raw);
        }
    }

    PooledRenderTarget* created = CreateNew(width, height, bindFlags, format, samples);
    if (created) {
        ++m_ActiveAcquireCount;
    }
    return PooledRTHandle(created);
}

void RenderTargetPool::Return(PooledRenderTarget* rt)
{
    if (!rt) {
        return;
    }
    rt->pool = this;
    m_FreeList.emplace_back(rt);
    if (m_ActiveAcquireCount > 0) {
        --m_ActiveAcquireCount;
    }
}

RenderTargetPool::PooledRTHandle RenderTargetPool::AcquireColor(RenderTargetPool& pool, int width, int height)
{
    return pool.Acquire(
        width, height, Diligent::BIND_RENDER_TARGET | Diligent::BIND_SHADER_RESOURCE, Diligent::TEX_FORMAT_RGBA8_UNORM);
}

RenderTargetPool::PooledRTHandle RenderTargetPool::AcquireColorMSAA(RenderTargetPool& pool, int width, int height, int samples)
{
    return pool.Acquire(
        width, height, Diligent::BIND_RENDER_TARGET, Diligent::TEX_FORMAT_RGBA8_UNORM, samples);
}

RenderTargetPool::PooledRTHandle RenderTargetPool::AcquireDepthStencil(RenderTargetPool& pool, int width, int height)
{
    return pool.Acquire(width, height, Diligent::BIND_DEPTH_STENCIL, Diligent::TEX_FORMAT_D24_UNORM_S8_UINT);
}

RenderTargetPool::PooledRTHandle RenderTargetPool::AcquireDepthStencilMSAA(RenderTargetPool& pool, int width, int height, int samples)
{
    return pool.Acquire(width, height, Diligent::BIND_DEPTH_STENCIL, Diligent::TEX_FORMAT_D24_UNORM_S8_UINT, samples);
}

RenderTargetPool::PooledRTHandle RenderTargetPool::AcquirePostprocess(RenderTargetPool& pool, int width, int height)
{
    return pool.Acquire(
        width, height, Diligent::BIND_RENDER_TARGET | Diligent::BIND_SHADER_RESOURCE, Diligent::TEX_FORMAT_RGBA8_UNORM);
}

} // namespace RmlDiligent
