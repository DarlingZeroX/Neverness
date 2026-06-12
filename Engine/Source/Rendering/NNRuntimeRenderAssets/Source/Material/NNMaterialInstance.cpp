// NNMaterialInstance.cpp — Material instance with parameter overrides

#include "../../Material/NNMaterialInstance.h"
#include "../../Cache/NNPipelineCache.h"
#include <cstring>
#include <algorithm>

namespace NN::Runtime::Assets
{
    NNMaterialInstance::NNMaterialInstance(NNRef<NNMaterial> base)
        : m_Base(std::move(base))
    {
    }

    // ========================================================================
    //  Parameter overrides
    // ========================================================================

    void NNMaterialInstance::SetFloat(const char* name, float value)
    {
        auto& p = GetOrCreateOverride(name, NNMaterialParamType::Float);
        p.Float = value;
    }

    void NNMaterialInstance::SetInt(const char* name, int value)
    {
        auto& p = GetOrCreateOverride(name, NNMaterialParamType::Int);
        p.Int = value;
    }

    void NNMaterialInstance::SetVector2(const char* name, float x, float y)
    {
        auto& p = GetOrCreateOverride(name, NNMaterialParamType::Vector2);
        p.Vec2[0] = x; p.Vec2[1] = y;
    }

    void NNMaterialInstance::SetVector3(const char* name, float x, float y, float z)
    {
        auto& p = GetOrCreateOverride(name, NNMaterialParamType::Vector3);
        p.Vec3[0] = x; p.Vec3[1] = y; p.Vec3[2] = z;
    }

    void NNMaterialInstance::SetVector4(const char* name, float x, float y, float z, float w)
    {
        auto& p = GetOrCreateOverride(name, NNMaterialParamType::Vector4);
        p.Vec4[0] = x; p.Vec4[1] = y; p.Vec4[2] = z; p.Vec4[3] = w;
    }

    void NNMaterialInstance::SetTexture(const char* name, NNRef<INNTexture> tex)
    {
        auto& p = GetOrCreateOverride(name, NNMaterialParamType::Texture);
        p.Texture = std::move(tex);
    }

    void NNMaterialInstance::SetSampler(const char* name, NNRef<INNSampler> sampler)
    {
        auto& p = GetOrCreateOverride(name, NNMaterialParamType::Sampler);
        p.Sampler = std::move(sampler);
    }

    // ========================================================================
    //  Get effective parameter (override > base)
    // ========================================================================

    const NNMaterialParam* NNMaterialInstance::GetParam(const char* name) const
    {
        // Check overrides first
        size_t idx = FindOverride(name);
        if (idx != SIZE_MAX)
        {
            return &m_Overrides[idx];
        }

        // Fall back to base material
        if (m_Base)
        {
            return m_Base->GetParam(name);
        }

        return nullptr;
    }

    // ========================================================================
    //  Apply material instance
    // ========================================================================

    void NNMaterialInstance::Apply(INNCommandList* cmd,
                                   NNPipelineCache* cache,
                                   const NNVertexLayout& vertexLayout,
                                   NNPixelFormat rtvFormat,
                                   NNPixelFormat dsvFormat,
                                   uint32_t sampleCount)
    {
        if (!m_Base) return;

        // Apply base material first (sets PSO)
        m_Base->Apply(cmd, cache, vertexLayout, rtvFormat, dsvFormat, sampleCount);

        // TODO: Apply overrides (bind override textures/samplers/constants)
        // This will override base material bindings in the backend
    }

    // ========================================================================
    //  Internal helpers
    // ========================================================================

    NNMaterialParam& NNMaterialInstance::GetOrCreateOverride(const char* name, NNMaterialParamType type)
    {
        size_t idx = FindOverride(name);
        if (idx != SIZE_MAX)
        {
            m_Overrides[idx].Type = type;
            return m_Overrides[idx];
        }

        m_Overrides.emplace_back();
        auto& p = m_Overrides.back();
        p.Name = name;
        p.Type = type;
        return p;
    }

    size_t NNMaterialInstance::FindOverride(const char* name) const
    {
        for (size_t i = 0; i < m_Overrides.size(); ++i)
        {
            if (m_Overrides[i].Name == name) return i;
        }
        return SIZE_MAX;
    }

} // namespace NN::Runtime::Assets
