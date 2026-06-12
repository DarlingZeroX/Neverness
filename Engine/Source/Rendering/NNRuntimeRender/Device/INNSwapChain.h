#pragma once

#include "../NNRenderConfig.h"
#include <NNRuntimeCore/NNObject.h>

namespace NN::Runtime::Render
{
    class INNSwapChain : public NN::Runtime::Core::INNObject
    {
    public:
        virtual void Present() = 0;
        virtual void Resize(uint32_t width, uint32_t height) = 0;
        virtual uint32_t GetCurrentBackBufferIndex() const = 0;
        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;
    };

} // namespace NN::Runtime::Render
