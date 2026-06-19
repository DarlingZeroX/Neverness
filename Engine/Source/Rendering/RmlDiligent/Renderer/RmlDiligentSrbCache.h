// =============================================================================
// RmlDiligentSrbCache.h
// PSO × 资源视图 SRB 缓存 — 避免全屏/滤镜 pass 每帧 CreateShaderResourceBinding
// =============================================================================
#pragma once

#include "Common/interface/RefCntAutoPtr.hpp"
#include "Graphics/GraphicsEngine/interface/Buffer.h"
#include "Graphics/GraphicsEngine/interface/PipelineState.h"
#include "Graphics/GraphicsEngine/interface/ShaderResourceBinding.h"
#include "Graphics/GraphicsEngine/interface/TextureView.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <unordered_map>

namespace RmlDiligent {

/// 全屏与滤镜路径的 SRB 二级缓存。键 = PSO 指针 + 主 SRV + 可选 CB + 次 SRV（BlendMask）。
class SrbCache {
public:
    using BindFn = std::function<void(Diligent::IShaderResourceBinding*)>;

    /// 获取或创建 SRB；cacheEnabled=false 时始终新建（A/B 对比用）。
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> GetOrCreate(
        Diligent::IPipelineState* pso,
        Diligent::ITextureView* primarySrv,
        Diligent::IBuffer* constantBuffer,
        bool cacheEnabled,
        const BindFn& bindFn,
        Diligent::ITextureView* secondarySrv = nullptr);

    /// 清空缓存（窗口 Resize / RT Pool 重建后 SRV 指针失效时调用）。
    void Clear();

    uint32_t GetHits() const { return m_Hits; }
    uint32_t GetMisses() const { return m_Misses; }
    size_t GetEntryCount() const { return m_Entries.size(); }
    void ResetStats();

private:
    static uint64_t MakeKey(
        Diligent::IPipelineState* pso,
        Diligent::ITextureView* srv,
        Diligent::IBuffer* cb,
        Diligent::ITextureView* secondarySrv);

    std::unordered_map<uint64_t, Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding>> m_Entries;
    uint32_t m_Hits = 0;
    uint32_t m_Misses = 0;
};

} // namespace RmlDiligent
