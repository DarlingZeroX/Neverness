#pragma once

#include "../NNRAssetsConfig.h"
#include "NNMaterial.h"
#include "NNMaterialParam.h"
#include <NNRuntimeCore/NNObject.h>
#include <vector>

namespace NN::Runtime::Assets
{
    using namespace NN::Runtime::Render;
    using namespace NN::Runtime::Core;

    // ========================================================================
    //  NNMaterialInstance — Material with parameter overrides
    //  Base material is shared, instance has its own parameter values
    // ========================================================================

    class NNMaterialInstance : public INNObject
    {
    public:
        explicit NNMaterialInstance(NNRef<NNMaterial> base);
        ~NNMaterialInstance() override = default;

        // === Base material ===
        NNRef<NNMaterial> GetBaseMaterial() const { return m_Base; }

        // === Parameter overrides (does not modify base) ===
        void SetFloat(const char* name, float value);
        void SetInt(const char* name, int value);
        void SetVector2(const char* name, float x, float y);
        void SetVector3(const char* name, float x, float y, float z);
        void SetVector4(const char* name, float x, float y, float z, float w);
        void SetTexture(const char* name, NNRef<INNTexture> tex);
        void SetSampler(const char* name, NNRef<INNSampler> sampler);

        // === Get effective parameter (override if exists, else base) ===
        const NNMaterialParam* GetParam(const char* name) const;

        // === Apply: merges base + overrides, then applies ===
        void Apply(INNCommandList* cmd,
                   NNPipelineCache* cache,
                   const NNVertexLayout& vertexLayout,
                   NNPixelFormat rtvFormat = NNPixelFormat::RGBA8_UNORM,
                   NNPixelFormat dsvFormat = NNPixelFormat::D32_FLOAT,
                   uint32_t sampleCount = 1);

        // === INNObject ===
        uint32_t AddRef() override { return ++m_RefCount; }
        uint32_t Release() override { uint32_t r = --m_RefCount; if (r == 0) delete this; return r; }
        uint32_t GetRefCount() const override { return m_RefCount; }

    private:
        NNRef<NNMaterial> m_Base;
        std::vector<NNMaterialParam> m_Overrides;
        std::atomic<uint32_t> m_RefCount{0};

        NNMaterialParam& GetOrCreateOverride(const char* name, NNMaterialParamType type);
        size_t FindOverride(const char* name) const;
    };

} // namespace NN::Runtime::Assets
