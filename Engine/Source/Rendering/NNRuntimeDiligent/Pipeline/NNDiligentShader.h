// NNDiligentShader.h -- Diligent shader wrapper

#pragma once

#include "../NNDiligentConfig.h"
#include <NNRuntimeRender/Pipeline/INNShader.h>
#include <NNRuntimeCore/NNObject.h>
#include <atomic>
#include <string>

namespace NNDiligent
{
    using namespace NN::Runtime::Core;
    using namespace NN::Runtime::Render;

    class NNDiligentShader : public INNShader
    {
    public:
        NNDiligentShader(::Diligent::IShader* shader, NNShaderStage stage, const char* debugName);
        ~NNDiligentShader() override;

        // INNShader
        NNShaderStage GetStage() const override;
        const char* GetDebugName() const override;

        // INNObject
        uint32_t AddRef() override;
        uint32_t Release() override;
        uint32_t GetRefCount() const override;

        // Internal: get the underlying Diligent shader
        ::Diligent::IShader* GetDiligentShader() const { return m_Shader; }

    private:
        ::Diligent::IShader* m_Shader = nullptr;
        NNShaderStage m_Stage;
        std::string m_DebugName;
        std::atomic<uint32_t> m_RefCount{0};
    };

} // namespace NNDiligent
