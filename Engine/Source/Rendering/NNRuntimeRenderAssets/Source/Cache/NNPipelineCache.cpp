// NNPipelineCache.cpp — PSO cache implementation

#include "../../Cache/NNPipelineCache.h"
#include <cstring>
#include <iostream>

namespace NN::Runtime::Assets
{
    // ========================================================================
    //  NNPipelineKey hash and comparison
    // ========================================================================

    uint64_t NNPipelineKey::Hash() const
    {
        uint64_t h = 0;

        // Hash pointers (shader identity)
        h ^= std::hash<const void*>{}(VS) + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= std::hash<const void*>{}(PS) + 0x9e3779b9 + (h << 6) + (h >> 2);

        // Hash vertex layout (stride + attribute count)
        h ^= std::hash<uint32_t>{}(VertexLayout.Stride) + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= std::hash<size_t>{}(VertexLayout.Attributes.size()) + 0x9e3779b9 + (h << 6) + (h >> 2);

        // Hash rasterizer state
        h ^= std::hash<uint8_t>{}(static_cast<uint8_t>(Rasterizer.FillMode)) + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= std::hash<uint8_t>{}(static_cast<uint8_t>(Rasterizer.CullMode)) + 0x9e3779b9 + (h << 6) + (h >> 2);

        // Hash blend state
        h ^= std::hash<bool>{}(Blend.Enable) + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= std::hash<uint8_t>{}(static_cast<uint8_t>(Blend.SrcBlend)) + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= std::hash<uint8_t>{}(static_cast<uint8_t>(Blend.DestBlend)) + 0x9e3779b9 + (h << 6) + (h >> 2);

        // Hash depth state
        h ^= std::hash<bool>{}(DepthStencil.DepthEnable) + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= std::hash<bool>{}(DepthStencil.DepthWriteEnable) + 0x9e3779b9 + (h << 6) + (h >> 2);

        // Hash formats
        h ^= std::hash<uint8_t>{}(static_cast<uint8_t>(RTVFormat)) + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= std::hash<uint8_t>{}(static_cast<uint8_t>(DSVFormat)) + 0x9e3779b9 + (h << 6) + (h >> 2);

        // Hash sample count
        h ^= std::hash<uint32_t>{}(SampleCount) + 0x9e3779b9 + (h << 6) + (h >> 2);

        // Hash topology
        h ^= std::hash<uint8_t>{}(static_cast<uint8_t>(Topology)) + 0x9e3779b9 + (h << 6) + (h >> 2);

        return h;
    }

    bool NNPipelineKey::operator==(const NNPipelineKey& other) const
    {
        if (VS != other.VS) return false;
        if (PS != other.PS) return false;
        if (VertexLayout.Stride != other.VertexLayout.Stride) return false;
        if (VertexLayout.Attributes.size() != other.VertexLayout.Attributes.size()) return false;
        if (Rasterizer.FillMode != other.Rasterizer.FillMode) return false;
        if (Rasterizer.CullMode != other.Rasterizer.CullMode) return false;
        if (Blend.Enable != other.Blend.Enable) return false;
        if (Blend.Enable)
        {
            if (Blend.SrcBlend != other.Blend.SrcBlend) return false;
            if (Blend.DestBlend != other.Blend.DestBlend) return false;
            if (Blend.BlendOp != other.Blend.BlendOp) return false;
        }
        if (DepthStencil.DepthEnable != other.DepthStencil.DepthEnable) return false;
        if (DepthStencil.DepthWriteEnable != other.DepthStencil.DepthWriteEnable) return false;
        if (DepthStencil.DepthFunc != other.DepthStencil.DepthFunc) return false;
        if (Topology != other.Topology) return false;
        if (RTVFormat != other.RTVFormat) return false;
        if (DSVFormat != other.DSVFormat) return false;
        if (SampleCount != other.SampleCount) return false;
        return true;
    }

    // ========================================================================
    //  NNPipelineCache implementation
    // ========================================================================

    NNPipelineCache::NNPipelineCache(INNRenderDevice* device)
        : m_Device(device)
    {
    }

    NNRef<INNPipelineState> NNPipelineCache::GetOrCreate(const NNPipelineKey& key)
    {
        if (!m_Device) return nullptr;

        uint64_t hash = key.Hash();

        // Check cache
        auto it = m_Cache.find(hash);
        if (it != m_Cache.end())
        {
            return it->second;
        }

        // Create new PSO
        NNPipelineStateDesc desc;
        desc.VS = key.VS;
        desc.PS = key.PS;
        desc.VertexLayout = key.VertexLayout;
        desc.RasterizerState = key.Rasterizer;
        desc.BlendState = key.Blend;
        desc.DepthStencilState = key.DepthStencil;
        desc.Topology = key.Topology;
        desc.RTVFormat = key.RTVFormat;
        desc.DSVFormat = key.DSVFormat;
        desc.SampleCount = key.SampleCount;

        auto pso = m_Device->CreatePipelineState(desc);
        if (!pso)
        {
            std::cerr << "[NNPipelineCache] Failed to create PSO (hash=0x"
                      << std::hex << hash << std::dec << ")" << std::endl;
            return nullptr;
        }

        m_Cache[hash] = pso;
        return pso;
    }

    void NNPipelineCache::Clear()
    {
        m_Cache.clear();
    }

} // namespace NN::Runtime::Assets
