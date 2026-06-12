#pragma once

#include "../NNRenderConfig.h"
#include <NNRuntimeCore/NNObject.h>
#include <cstdint>

namespace NN::Runtime::Render
{
    enum class NNBufferUsage : uint8_t
    {
        Static, Dynamic, Staging
    };

    enum class NNBufferType : uint8_t
    {
        Vertex, Index, Constant, Storage
    };

    struct NNBufferDesc
    {
        NNBufferType Type = NNBufferType::Vertex;
        NNBufferUsage Usage = NNBufferUsage::Static;
        uint32_t Size = 0;
        uint32_t Stride = 0;
        bool CPUAccessible = false;
    };

    class INNBuffer : public NN::Runtime::Core::INNObject
    {
    public:
        virtual const NNBufferDesc& GetDesc() const = 0;
        virtual void UpdateData(const void* data, uint32_t size, uint32_t offset = 0) = 0;
        virtual uint32_t GetSize() const = 0;
    };

} // namespace NN::Runtime::Render
