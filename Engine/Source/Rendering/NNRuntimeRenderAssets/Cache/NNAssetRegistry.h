#pragma once

#include "../NNRAssetsConfig.h"
#include "../Shader/NNShaderAsset.h"
#include "../Material/NNMaterial.h"
#include "../Material/NNMaterialInstance.h"
#include <NNRuntimeCore/Handle/NNHandleTypes.h>
#include <NNRuntimeCore/NNObject.h>
#include <unordered_map>

namespace NN::Runtime::Assets
{
    using namespace NN::Runtime::Core;

    // ========================================================================
    //  NNAssetRegistry — Asset handle registry
    //  Maps handles to shader assets, materials, and material instances
    //  Used by NNNativeEngineAPI for C# interop
    // ========================================================================

    class NNAssetRegistry : public INNObject
    {
    public:
        NNAssetRegistry();
        ~NNAssetRegistry() override = default;

        // === Register assets ===
        NNRenderHandle RegisterShaderAsset(NNRef<NNShaderAsset> asset);
        NNRenderHandle RegisterMaterial(NNRef<NNMaterial> material);
        NNRenderHandle RegisterMaterialInstance(NNRef<NNMaterialInstance> instance);

        // === Get assets by handle ===
        NNShaderAsset* GetShaderAsset(NNRenderHandle handle) const;
        NNMaterial* GetMaterial(NNRenderHandle handle) const;
        NNMaterialInstance* GetMaterialInstance(NNRenderHandle handle) const;

        // === Release handle ===
        void Release(NNRenderHandle handle);

        // === Stats ===
        size_t GetShaderAssetCount() const { return m_ShaderAssets.size(); }
        size_t GetMaterialCount() const { return m_Materials.size(); }
        size_t GetMaterialInstanceCount() const { return m_MaterialInstances.size(); }
        size_t GetTotalCount() const;

        // === INNObject ===
        uint32_t AddRef() override { return ++m_RefCount; }
        uint32_t Release() override { uint32_t r = --m_RefCount; if (r == 0) delete this; return r; }
        uint32_t GetRefCount() const override { return m_RefCount; }

    private:
        // Separate maps for type safety
        std::unordered_map<NNRenderHandle, NNRef<NNShaderAsset>> m_ShaderAssets;
        std::unordered_map<NNRenderHandle, NNRef<NNMaterial>> m_Materials;
        std::unordered_map<NNRenderHandle, NNRef<NNMaterialInstance>> m_MaterialInstances;

        // Handle generation
        uint32_t m_NextIndex = 1;
        std::atomic<uint32_t> m_RefCount{0};

        NNRenderHandle MakeAssetHandle(NNHandleType type, uint32_t index);
    };

} // namespace NN::Runtime::Assets
