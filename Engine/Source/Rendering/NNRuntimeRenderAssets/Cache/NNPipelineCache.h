#pragma once

#include "../NNRAssetsConfig.h"
#include <NNRuntimeRender/Pipeline/INNPipelineState.h>
#include <NNRuntimeRender/Pipeline/INNShader.h>
#include <NNRuntimeRender/Device/INNRenderDevice.h>
#include <NNRuntimeCore/NNObject.h>
#include <unordered_map>
#include <functional>

namespace NN::Runtime::Assets
{
    using namespace NN::Runtime::Render;
    using namespace NN::Runtime::Core;

    // ========================================================================
    //  NNPipelineKey — Uniquely identifies a PSO configuration
    // ========================================================================

    struct NNPipelineKey
    {
        INNShader* VS = nullptr;
        INNShader* PS = nullptr;
        NNVertexLayout VertexLayout;
        NNRasterizerState Rasterizer;
        NNBlendState Blend;
        NNDepthStencilState DepthStencil;
        NNPrimitiveTopology Topology = NNPrimitiveTopology::TriangleList;
        NNPixelFormat RTVFormat = NNPixelFormat::RGBA8_UNORM;
        NNPixelFormat DSVFormat = NNPixelFormat::D32_FLOAT;
        uint32_t SampleCount = 1;

        uint64_t Hash() const;
        bool operator==(const NNPipelineKey& other) const;
    };

    // ========================================================================
    //  NNPipelineCache — PSO cache (one per device)
    //  Caches PSOs to avoid redundant creation
    // ========================================================================

    class NNPipelineCache : public INNObject
    {
    public:
        explicit NNPipelineCache(INNRenderDevice* device);
        ~NNPipelineCache() override = default;

        // === Get or create PSO ===
        NNRef<INNPipelineState> GetOrCreate(const NNPipelineKey& key);

        // === Clear all cached PSOs ===
        void Clear();

        // === Stats ===
        size_t GetCount() const { return m_Cache.size(); }

        // === Get device ===
        INNRenderDevice* GetDevice() const { return m_Device; }

        // === INNObject ===
        uint32_t AddRef() override { return ++m_RefCount; }
        uint32_t Release() override { uint32_t r = --m_RefCount; if (r == 0) delete this; return r; }
        uint32_t GetRefCount() const override { return m_RefCount; }

    private:
        INNRenderDevice* m_Device;
        std::unordered_map<uint64_t, NNRef<INNPipelineState>> m_Cache;
        std::atomic<uint32_t> m_RefCount{0};
    };

} // namespace NN::Runtime::Assets
