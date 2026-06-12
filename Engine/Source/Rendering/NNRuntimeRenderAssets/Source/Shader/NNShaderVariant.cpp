// NNShaderVariant.cpp — Shader variant implementation

#include "../../Shader/NNShaderVariant.h"
#include <iostream>

namespace NN::Runtime::Assets
{
    NNShaderVariant::NNShaderVariant(NNRef<NNShaderAsset> asset, const NNShaderVariantKey& key)
        : m_Asset(std::move(asset))
        , m_Key(key)
    {
    }

    NNRef<INNShader> NNShaderVariant::GetShader(INNRenderDevice* device)
    {
        if (!m_Asset) return nullptr;

        // Return cached if available
        if (m_CachedShader)
        {
            return m_CachedShader;
        }

        // TODO: For now, just compile the base asset directly
        // Future: inject variant defines (#define FEATURE_MASK, #define MATERIAL_FLAGS)
        //         into shader source before compilation
        m_CachedShader = m_Asset->CreateShader(device);

        if (!m_CachedShader)
        {
            std::cerr << "[NNShaderVariant] Failed to compile variant: "
                      << m_Asset->GetName()
                      << " (features=0x" << std::hex << m_Key.FeatureMask
                      << " material=0x" << m_Key.MaterialFlags << std::dec << ")"
                      << std::endl;
        }

        return m_CachedShader;
    }

} // namespace NN::Runtime::Assets
