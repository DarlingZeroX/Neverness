#pragma once

#include <cstdint>
#include <string>
#include "NNRenderConfig.h"

namespace NN::Runtime::Render
{
    struct NNDeviceInfo
    {
        NNRenderBackendType Backend;
        const char* DeviceName;
        uint64_t MaxTextureSize;
        uint64_t MaxBufferSize;
        uint32_t MaxTextureArrayLayers;
        uint32_t MaxColorAttachments;
        bool SupportsCompute;
        bool SupportsRayTracing;
        bool SupportsGeometryShader;
        bool SupportsTessellation;
    };

    enum class NNFeature
    {
        Compute,
        RayTracing,
        GeometryShader,
        Tessellation,
        MultiDrawIndirect,
        TextureCompressionBC,
        TextureCompressionASTC,
    };

    struct NNSwapChainDesc
    {
        uint32_t Width;
        uint32_t Height;
        uint32_t BufferCount;
        void* Window;
        bool VSync;
        bool Fullscreen;
    };

    struct NNRenderDeviceCreateInfo
    {
        NNRenderBackendType Backend = NNRenderBackendType::Auto;
        void* Window = nullptr;
        uint32_t Width = 1280;
        uint32_t Height = 720;
        bool EnableValidation = true;
        bool VSync = true;
    };

} // namespace NN::Runtime::Render
