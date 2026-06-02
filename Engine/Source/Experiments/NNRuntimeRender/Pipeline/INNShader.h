#pragma once

#include "../NNRenderConfig.h"
#include <NNRuntimeCore/NNObject.h>
#include <cstdint>
#include <string>
#include <vector>

namespace NN::Runtime::Render
{
    enum class NNShaderStage : uint8_t
    {
        Vertex, Pixel, Geometry, Hull, Domain, Compute
    };

    enum class NNShaderLanguage : uint8_t
    {
        HLSL, GLSL, SPIRV
    };

    struct NNShaderDesc
    {
        NNShaderStage Stage;
        NNShaderLanguage Language = NNShaderLanguage::HLSL;
        const char* SourceCode = nullptr;
        uint32_t SourceLength = 0;
        const uint32_t* ByteCode = nullptr;
        uint32_t ByteCodeSize = 0;
        const char* EntryPoint = "main";
        const char* DebugName = nullptr;
    };

    class INNShader : public NN::Runtime::Core::INNObject
    {
    public:
        virtual NNShaderStage GetStage() const = 0;
        virtual const char* GetDebugName() const = 0;
    };

} // namespace NN::Runtime::Render
