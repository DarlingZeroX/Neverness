#include "RmlDiligentSrbCache.h"

namespace RmlDiligent {

uint64_t SrbCache::MakeKey(
    Diligent::IPipelineState* pso,
    Diligent::ITextureView* srv,
    Diligent::IBuffer* cb,
    Diligent::ITextureView* secondarySrv)
{
    const uint64_t p = reinterpret_cast<uint64_t>(pso);
    const uint64_t s = reinterpret_cast<uint64_t>(srv);
    const uint64_t c = reinterpret_cast<uint64_t>(cb);
    const uint64_t s2 = reinterpret_cast<uint64_t>(secondarySrv);
    return (p << 32) ^ (s << 1) ^ c ^ (s2 << 16);
}

Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> SrbCache::GetOrCreate(
    Diligent::IPipelineState* pso,
    Diligent::ITextureView* primarySrv,
    Diligent::IBuffer* constantBuffer,
    bool cacheEnabled,
    const BindFn& bindFn,
    Diligent::ITextureView* secondarySrv)
{
    if (!pso || !bindFn) {
        return {};
    }

    if (cacheEnabled) {
        const uint64_t key = MakeKey(pso, primarySrv, constantBuffer, secondarySrv);
        if (auto it = m_Entries.find(key); it != m_Entries.end()) {
            ++m_Hits;
            return it->second;
        }
    }

    ++m_Misses;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> srb;
    pso->CreateShaderResourceBinding(&srb, true);
    if (!srb) {
        return {};
    }
    bindFn(srb);

    if (cacheEnabled) {
        const uint64_t key = MakeKey(pso, primarySrv, constantBuffer, secondarySrv);
        m_Entries[key] = srb;
    }
    return srb;
}

void SrbCache::Clear()
{
    m_Entries.clear();
}

void SrbCache::ResetStats()
{
    m_Hits = 0;
    m_Misses = 0;
}

} // namespace RmlDiligent
