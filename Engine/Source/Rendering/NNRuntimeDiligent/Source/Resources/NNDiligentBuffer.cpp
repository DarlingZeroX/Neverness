// NNDiligentBuffer.cpp — Diligent Buffer wrapper

#include "../../Resources/NNDiligentBuffer.h"
#include <cstring>

namespace NNDiligent
{
    NNDiligentBuffer::NNDiligentBuffer(::Diligent::IBuffer* buffer, const NNBufferDesc& desc)
        : m_Buffer(buffer)
        , m_Desc(desc)
        , m_Size(desc.Size)
    {
        if (m_Buffer) m_Buffer->AddRef();
    }

    NNDiligentBuffer::~NNDiligentBuffer()
    {
        if (m_Buffer)
        {
            m_Buffer->Release();
            m_Buffer = nullptr;
        }
    }

    const NNBufferDesc& NNDiligentBuffer::GetDesc() const
    {
        return m_Desc;
    }

    void NNDiligentBuffer::UpdateData(const void* data, uint32_t size, uint32_t offset)
    {
        if (!m_Buffer || !data || size == 0)
            return;

        if (m_Desc.Usage == NNBufferUsage::Dynamic && m_Context)
        {
            // Dynamic buffer: use Map/UnMap
            void* mappedData = nullptr;
            ::Diligent::MAP_FLAGS mapFlags = ::Diligent::MAP_FLAG_DISCARD;
            m_Context->MapBuffer(m_Buffer, ::Diligent::MAP_WRITE, mapFlags, mappedData);
            if (mappedData)
            {
                auto* dst = static_cast<uint8_t*>(mappedData) + offset;
                std::memcpy(dst, data, size);
                m_Context->UnmapBuffer(m_Buffer, ::Diligent::MAP_WRITE);
            }
        }
        else if (m_Context)
        {
            // Static/Staging: use UpdateBuffer
            m_Context->UpdateBuffer(m_Buffer, offset, size, data, ::Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        }
    }

    uint32_t NNDiligentBuffer::GetSize() const
    {
        return m_Size;
    }

    uint32_t NNDiligentBuffer::AddRef()
    {
        return ++m_RefCount;
    }

    uint32_t NNDiligentBuffer::Release()
    {
        uint32_t c = --m_RefCount;
        if (c == 0)
            delete this;
        return c;
    }

    uint32_t NNDiligentBuffer::GetRefCount() const
    {
        return m_RefCount;
    }

} // namespace NNDiligent
