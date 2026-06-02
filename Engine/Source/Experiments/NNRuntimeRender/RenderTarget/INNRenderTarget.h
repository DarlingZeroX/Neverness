#pragma once

#include "../NNRenderConfig.h"
#include "../Resources/INNTexture.h"
#include <NNRuntimeCore/NNObject.h>
#include <cstdint>
#include <vector>

namespace NN::Runtime::Render
{
    struct NNRenderTargetDesc
    {
        uint32_t Width = 1;
        uint32_t Height = 1;
        NNPixelFormat ColorFormat = NNPixelFormat::RGBA8_UNORM;
        NNPixelFormat DepthFormat = NNPixelFormat::D32_FLOAT;
        uint32_t ColorAttachmentCount = 1;
        uint32_t SampleCount = 1;
        const char* DebugName = nullptr;
    };

    class INNRenderTarget : public NN::Runtime::Core::INNObject
    {
    public:
        virtual const NNRenderTargetDesc& GetDesc() const = 0;
        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;
    };

} // namespace NN::Runtime::Render
