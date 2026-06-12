#pragma once

#include <NNRuntimeRender/Resources/INNTexture.h>
#include <NNRuntimeRender/Resources/INNSampler.h>
#include <NNRuntimeCore/NNObject.h>
#include <string>
#include <cstring>

namespace NN::Runtime::Assets
{
    using namespace NN::Runtime::Render;
    using namespace NN::Runtime::Core;

    // ========================================================================
    //  Material parameter types
    // ========================================================================

    enum class NNMaterialParamType : uint8_t
    {
        Float,
        Int,
        Vector2,
        Vector3,
        Vector4,
        Texture,
        Sampler
    };

    // ========================================================================
    //  Material parameter — single named value
    // ========================================================================

    struct NNMaterialParam
    {
        std::string Name;
        NNMaterialParamType Type;

        union
        {
            float Float;
            int Int;
            float Vec2[2];
            float Vec3[3];
            float Vec4[4];
        };

        NNRef<INNTexture> Texture;
        NNRef<INNSampler> Sampler;

        NNMaterialParam()
            : Type(NNMaterialParamType::Float)
            , Float(0.0f)
        {
        }

        NNMaterialParam(const char* name, float value)
            : Name(name)
            , Type(NNMaterialParamType::Float)
            , Float(value)
        {
        }

        NNMaterialParam(const char* name, int value)
            : Name(name)
            , Type(NNMaterialParamType::Int)
            , Int(value)
        {
        }

        NNMaterialParam(const char* name, float x, float y, float z, float w)
            : Name(name)
            , Type(NNMaterialParamType::Vector4)
        {
            Vec4[0] = x; Vec4[1] = y; Vec4[2] = z; Vec4[3] = w;
        }

        NNMaterialParam(const char* name, NNRef<INNTexture> tex)
            : Name(name)
            , Type(NNMaterialParamType::Texture)
            , Texture(std::move(tex))
        {
        }

        NNMaterialParam(const char* name, NNRef<INNSampler> sampler)
            : Name(name)
            , Type(NNMaterialParamType::Sampler)
            , Sampler(std::move(sampler))
        {
        }
    };

} // namespace NN::Runtime::Assets
