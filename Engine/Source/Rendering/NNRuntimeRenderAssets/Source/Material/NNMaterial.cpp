// NNMaterial.cpp — Material implementation

#include "../../Material/NNMaterial.h"
#include "../../Cache/NNPipelineCache.h"
#include <cstring>
#include <algorithm>

namespace NN::Runtime::Assets
{
    NNMaterial::NNMaterial()
    {
        // Default state
        m_Rasterizer.FillMode = NNFillMode::Solid;
        m_Rasterizer.CullMode = NNCullMode::Back;

        m_Blend.Enable = false;
        m_Blend.SrcBlend = NNBlendFactor::One;
        m_Blend.DestBlend = NNBlendFactor::Zero;
        m_Blend.BlendOp = NNBlendOp::Add;

        m_Depth.DepthEnable = true;
        m_Depth.DepthWriteEnable = true;
        m_Depth.DepthFunc = NNCompareFunc::LessEqual;

        m_Topology = NNPrimitiveTopology::TriangleList;
    }

    // ========================================================================
    //  Shader setup
    // ========================================================================

    void NNMaterial::SetVertexShader(NNRef<INNShader> vs)
    {
        m_VS = std::move(vs);
    }

    void NNMaterial::SetPixelShader(NNRef<INNShader> ps)
    {
        m_PS = std::move(ps);
    }

    // ========================================================================
    //  Parameter setters
    // ========================================================================

    void NNMaterial::SetFloat(const char* name, float value)
    {
        auto& p = GetOrCreateParam(name, NNMaterialParamType::Float);
        p.Float = value;
    }

    void NNMaterial::SetInt(const char* name, int value)
    {
        auto& p = GetOrCreateParam(name, NNMaterialParamType::Int);
        p.Int = value;
    }

    void NNMaterial::SetVector2(const char* name, float x, float y)
    {
        auto& p = GetOrCreateParam(name, NNMaterialParamType::Vector2);
        p.Vec2[0] = x; p.Vec2[1] = y;
    }

    void NNMaterial::SetVector3(const char* name, float x, float y, float z)
    {
        auto& p = GetOrCreateParam(name, NNMaterialParamType::Vector3);
        p.Vec3[0] = x; p.Vec3[1] = y; p.Vec3[2] = z;
    }

    void NNMaterial::SetVector4(const char* name, float x, float y, float z, float w)
    {
        auto& p = GetOrCreateParam(name, NNMaterialParamType::Vector4);
        p.Vec4[0] = x; p.Vec4[1] = y; p.Vec4[2] = z; p.Vec4[3] = w;
    }

    void NNMaterial::SetTexture(const char* name, NNRef<INNTexture> tex)
    {
        auto& p = GetOrCreateParam(name, NNMaterialParamType::Texture);
        p.Texture = std::move(tex);
    }

    void NNMaterial::SetSampler(const char* name, NNRef<INNSampler> sampler)
    {
        auto& p = GetOrCreateParam(name, NNMaterialParamType::Sampler);
        p.Sampler = std::move(sampler);
    }

    // ========================================================================
    //  Parameter getters
    // ========================================================================

    const NNMaterialParam* NNMaterial::GetParam(const char* name) const
    {
        for (const auto& p : m_Params)
        {
            if (p.Name == name) return &p;
        }
        return nullptr;
    }

    // ========================================================================
    //  Blend state
    // ========================================================================

    void NNMaterial::SetBlendFunc(NNBlendFactor src, NNBlendFactor dst, NNBlendOp op)
    {
        m_Blend.SrcBlend = src;
        m_Blend.DestBlend = dst;
        m_Blend.BlendOp = op;
    }

    // ========================================================================
    //  Apply material to command list
    // ========================================================================

    void NNMaterial::Apply(INNCommandList* cmd,
                           NNPipelineCache* cache,
                           const NNVertexLayout& vertexLayout,
                           NNPixelFormat rtvFormat,
                           NNPixelFormat dsvFormat,
                           uint32_t sampleCount)
    {
        if (!cmd || !cache) return;
        if (!m_VS || !m_PS) return;

        // Build pipeline key
        NNPipelineKey key;
        key.VS = m_VS.Get();
        key.PS = m_PS.Get();
        key.VertexLayout = vertexLayout;
        key.Rasterizer = m_Rasterizer;
        key.Blend = m_Blend;
        key.DepthStencil = m_Depth;
        key.Topology = m_Topology;
        key.RTVFormat = rtvFormat;
        key.DSVFormat = dsvFormat;
        key.SampleCount = sampleCount;

        // Get or create PSO from cache
        auto pso = cache->GetOrCreate(key);
        if (!pso) return;

        // Set pipeline state
        cmd->SetPipelineState(pso.Get());

        // TODO: Set resource bindings (textures, samplers, constant buffers)
        // This will be implemented when Diligent SRB integration is added
        // For now, PSO is set — actual binding happens in backend
    }

    // ========================================================================
    //  Internal helpers
    // ========================================================================

    NNMaterialParam& NNMaterial::GetOrCreateParam(const char* name, NNMaterialParamType type)
    {
        for (auto& p : m_Params)
        {
            if (p.Name == name)
            {
                p.Type = type;
                return p;
            }
        }
        m_Params.emplace_back();
        auto& p = m_Params.back();
        p.Name = name;
        p.Type = type;
        return p;
    }

} // namespace NN::Runtime::Assets
