// NNDiligentShader.cpp -- Diligent shader wrapper implementation

#include "../../Pipeline/NNDiligentShader.h"

namespace NNDiligent
{
    NNDiligentShader::NNDiligentShader(::Diligent::IShader* shader, NNShaderStage stage, const char* debugName)
        : m_Shader(shader)
        , m_Stage(stage)
        , m_DebugName(debugName ? debugName : "NNShader")
    {
    }

    NNDiligentShader::~NNDiligentShader()
    {
        if (m_Shader)
        {
            m_Shader->Release();
            m_Shader = nullptr;
        }
    }

    NNShaderStage NNDiligentShader::GetStage() const { return m_Stage; }
    const char* NNDiligentShader::GetDebugName() const { return m_DebugName.c_str(); }

    uint32_t NNDiligentShader::AddRef() { return ++m_RefCount; }
    uint32_t NNDiligentShader::Release()
    {
        uint32_t c = --m_RefCount;
        if (c == 0) delete this;
        return c;
    }
    uint32_t NNDiligentShader::GetRefCount() const { return m_RefCount; }

} // namespace NNDiligent
