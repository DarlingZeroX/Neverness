#pragma once

#include "../NNRAssetsConfig.h"
#include "NNMaterialParam.h"
#include <NNRuntimeRender/Pipeline/INNShader.h>
#include <NNRuntimeRender/Pipeline/INNPipelineState.h>
#include <NNRuntimeRender/Command/INNCommandList.h>
#include <NNRuntimeRender/Resources/INNTexture.h>
#include <NNRuntimeRender/Resources/INNSampler.h>
#include <NNRuntimeCore/NNObject.h>
#include <vector>
#include <string>

namespace NN::Runtime::Assets
{
    using namespace NN::Runtime::Render;
    using namespace NN::Runtime::Core;

    // Forward declaration
    class NNPipelineCache;

    // ========================================================================
    //  NNMaterial — Material with shader + parameters
    //  C# uses via Handle, C++ uses NNRef internally
    // ========================================================================

    class NNMaterial : public INNObject
    {
    public:
        NNMaterial();
        ~NNMaterial() override = default;

        // === Shader setup ===
        void SetVertexShader(NNRef<INNShader> vs);
        void SetPixelShader(NNRef<INNShader> ps);
        NNRef<INNShader> GetVertexShader() const { return m_VS; }
        NNRef<INNShader> GetPixelShader() const { return m_PS; }

        // === Parameter setters ===
        void SetFloat(const char* name, float value);
        void SetInt(const char* name, int value);
        void SetVector2(const char* name, float x, float y);
        void SetVector3(const char* name, float x, float y, float z);
        void SetVector4(const char* name, float x, float y, float z, float w);
        void SetTexture(const char* name, NNRef<INNTexture> tex);
        void SetSampler(const char* name, NNRef<INNSampler> sampler);

        // === Parameter getters ===
        const NNMaterialParam* GetParam(const char* name) const;
        const std::vector<NNMaterialParam>& GetParams() const { return m_Params; }
        size_t GetParamCount() const { return m_Params.size(); }

        // === Rasterizer state ===
        void SetFillMode(NNFillMode mode) { m_Rasterizer.FillMode = mode; }
        void SetCullMode(NNCullMode mode) { m_Rasterizer.CullMode = mode; }

        // === Blend state ===
        void SetBlendEnable(bool enable) { m_Blend.Enable = enable; }
        void SetBlendFunc(NNBlendFactor src, NNBlendFactor dst, NNBlendOp op);

        // === Depth state ===
        void SetDepthEnable(bool enable) { m_Depth.DepthEnable = enable; }
        void SetDepthWrite(bool enable) { m_Depth.DepthWriteEnable = enable; }

        // === Topology ===
        void SetTopology(NNPrimitiveTopology topo) { m_Topology = topo; }

        // === Apply material to command list ===
        // Internally handles PSO creation/caching and resource binding
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
        NNRef<INNShader> m_VS;
        NNRef<INNShader> m_PS;
        std::vector<NNMaterialParam> m_Params;

        NNRasterizerState m_Rasterizer;
        NNBlendState m_Blend;
        NNDepthStencilState m_Depth;
        NNPrimitiveTopology m_Topology = NNPrimitiveTopology::TriangleList;

        std::atomic<uint32_t> m_RefCount{0};

        // Find existing param or create new
        NNMaterialParam& GetOrCreateParam(const char* name, NNMaterialParamType type);
    };

} // namespace NN::Runtime::Assets
