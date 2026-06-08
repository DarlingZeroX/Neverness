#pragma once

#include "../NNRAssetsConfig.h"
#include "NNShaderAsset.h"
#include <NNRuntimeRender/Pipeline/INNShader.h>
#include <NNRuntimeRender/Device/INNRenderDevice.h>
#include <NNRuntimeCore/NNObject.h>
#include <cstdint>

namespace NN::Runtime::Assets
{
    using namespace NN::Runtime::Render;
    using namespace NN::Runtime::Core;

    // ========================================================================
    //  NNShaderVariantKey — Uniquely identifies a shader permutation
    // ========================================================================

    struct NNShaderVariantKey
    {
        uint32_t FeatureMask = 0;   // Engine features (e.g., fog, skinning)
        uint32_t MaterialFlags = 0; // Material-specific flags

        uint64_t Hash() const
        {
            // Simple combine hash
            uint64_t h = 0;
            h ^= std::hash<uint32_t>{}(FeatureMask) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= std::hash<uint32_t>{}(MaterialFlags) + 0x9e3779b9 + (h << 6) + (h >> 2);
            return h;
        }

        bool operator==(const NNShaderVariantKey& other) const
        {
            return FeatureMask == other.FeatureMask && MaterialFlags == other.MaterialFlags;
        }
    };

    // ========================================================================
    //  NNShaderVariant — A specific permutation of a shader asset
    //  Compiles on first access, caches the result
    // ========================================================================

    class NNShaderVariant : public INNObject
    {
    public:
        NNShaderVariant(NNRef<NNShaderAsset> asset, const NNShaderVariantKey& key);
        ~NNShaderVariant() override = default;

        // Get compiled shader (compiles on first access)
        NNRef<INNShader> GetShader(INNRenderDevice* device);

        // Get key
        const NNShaderVariantKey& GetKey() const { return m_Key; }

        // Get source asset
        NNRef<NNShaderAsset> GetAsset() const { return m_Asset; }

        // Check if compiled
        bool IsCompiled() const { return m_CachedShader != nullptr; }

        // === INNObject ===
        uint32_t AddRef() override { return ++m_RefCount; }
        uint32_t Release() override { uint32_t r = --m_RefCount; if (r == 0) delete this; return r; }
        uint32_t GetRefCount() const override { return m_RefCount; }

    private:
        NNRef<NNShaderAsset> m_Asset;
        NNShaderVariantKey m_Key;
        NNRef<INNShader> m_CachedShader;
        std::atomic<uint32_t> m_RefCount{0};
    };

} // namespace NN::Runtime::Assets
