#pragma once

#include "Common/interface/RefCntAutoPtr.hpp"
#include "Graphics/GraphicsEngine/interface/RenderDevice.h"
#include "Graphics/GraphicsEngine/interface/Texture.h"
#include "Graphics/GraphicsEngine/interface/TextureView.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace RmlDiligent {

class RenderTargetPool;

/// Pool 内单张 RT：color/depth/postprocess 共用此结构，Return 时归还空闲列表。
struct PooledRenderTarget {
    RenderTargetPool* pool = nullptr;
    Diligent::RefCntAutoPtr<Diligent::ITexture> texture;
    Diligent::RefCntAutoPtr<Diligent::ITextureView> RTV;
    Diligent::RefCntAutoPtr<Diligent::ITextureView> SRV;
    Diligent::RefCntAutoPtr<Diligent::ITextureView> DSV;
    int width = 0;
    int height = 0;
    Diligent::BIND_FLAGS bindFlags = Diligent::BIND_NONE;
    Diligent::TEXTURE_FORMAT format = Diligent::TEX_FORMAT_UNKNOWN;
};

/// 按尺寸/格式/BindFlags 复用 RT，减少 Composite/Filter 路径的 CreateTexture 开销。
class RenderTargetPool {
public:
    struct Deleter {
        void operator()(PooledRenderTarget* rt) const;
    };

    using PooledRTHandle = std::unique_ptr<PooledRenderTarget, Deleter>;

    void Initialize(Diligent::IRenderDevice* device);
    void OnResize(int width, int height);

    PooledRTHandle Acquire(int width, int height, Diligent::BIND_FLAGS bindFlags, Diligent::TEXTURE_FORMAT format);
    void Return(PooledRenderTarget* rt);

    static PooledRTHandle AcquireColor(RenderTargetPool& pool, int width, int height);
    static PooledRTHandle AcquireDepthStencil(RenderTargetPool& pool, int width, int height);
    static PooledRTHandle AcquirePostprocess(RenderTargetPool& pool, int width, int height);

    /// 当前空闲池中可复用 RT 数量（Phase 6 内存统计）
    size_t GetFreeListSize() const { return m_FreeList.size(); }
    /// 当前已 Acquire 尚未 Return 的 RT 数量
    size_t GetActiveAcquireCount() const { return m_ActiveAcquireCount; }

private:
    PooledRenderTarget* CreateNew(int width, int height, Diligent::BIND_FLAGS bindFlags, Diligent::TEXTURE_FORMAT format);

    Diligent::IRenderDevice* m_Device = nullptr;
    int m_Width = 0;
    int m_Height = 0;
    size_t m_ActiveAcquireCount = 0;
    std::vector<std::unique_ptr<PooledRenderTarget>> m_FreeList;
};

} // namespace RmlDiligent
