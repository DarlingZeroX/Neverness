// NNAssetRegistry.cpp — Asset handle registry implementation

#include "../../Cache/NNAssetRegistry.h"
#include <iostream>

namespace NN::Runtime::Assets
{
    NNAssetRegistry::NNAssetRegistry()
        : m_NextIndex(1)
    {
    }

    // ========================================================================
    //  Handle creation
    // ========================================================================

    NNRenderHandle NNAssetRegistry::MakeAssetHandle(NNHandleType type, uint32_t index)
    {
        return MakeHandle(type, index, 1);
    }

    // ========================================================================
    //  Register assets
    // ========================================================================

    NNRenderHandle NNAssetRegistry::RegisterShaderAsset(NNRef<NNShaderAsset> asset)
    {
        if (!asset) return NN_INVALID_HANDLE;

        uint32_t idx = m_NextIndex++;
        NNRenderHandle handle = MakeAssetHandle(NNHandleType::Shader, idx);
        m_ShaderAssets[handle] = std::move(asset);
        return handle;
    }

    NNRenderHandle NNAssetRegistry::RegisterMaterial(NNRef<NNMaterial> material)
    {
        if (!material) return NN_INVALID_HANDLE;

        uint32_t idx = m_NextIndex++;
        NNRenderHandle handle = MakeAssetHandle(NNHandleType::Material, idx);
        m_Materials[handle] = std::move(material);
        return handle;
    }

    NNRenderHandle NNAssetRegistry::RegisterMaterialInstance(NNRef<NNMaterialInstance> instance)
    {
        if (!instance) return NN_INVALID_HANDLE;

        uint32_t idx = m_NextIndex++;
        NNRenderHandle handle = MakeAssetHandle(NNHandleType::Material, idx);
        m_MaterialInstances[handle] = std::move(instance);
        return handle;
    }

    // ========================================================================
    //  Get assets by handle
    // ========================================================================

    NNShaderAsset* NNAssetRegistry::GetShaderAsset(NNRenderHandle handle) const
    {
        auto it = m_ShaderAssets.find(handle);
        if (it != m_ShaderAssets.end())
        {
            return it->second.Get();
        }
        return nullptr;
    }

    NNMaterial* NNAssetRegistry::GetMaterial(NNRenderHandle handle) const
    {
        auto it = m_Materials.find(handle);
        if (it != m_Materials.end())
        {
            return it->second.Get();
        }
        return nullptr;
    }

    NNMaterialInstance* NNAssetRegistry::GetMaterialInstance(NNRenderHandle handle) const
    {
        auto it = m_MaterialInstances.find(handle);
        if (it != m_MaterialInstances.end())
        {
            return it->second.Get();
        }
        return nullptr;
    }

    // ========================================================================
    //  Release handle
    // ========================================================================

    void NNAssetRegistry::Release(NNRenderHandle handle)
    {
        // Try each map
        if (m_ShaderAssets.erase(handle) > 0) return;
        if (m_Materials.erase(handle) > 0) return;
        if (m_MaterialInstances.erase(handle) > 0) return;
    }

    // ========================================================================
    //  Stats
    // ========================================================================

    size_t NNAssetRegistry::GetTotalCount() const
    {
        return m_ShaderAssets.size() + m_Materials.size() + m_MaterialInstances.size();
    }

} // namespace NN::Runtime::Assets
