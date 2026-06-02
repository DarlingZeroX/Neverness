#pragma once

#define NNRENDER_API

namespace NN::Runtime::Render
{
    enum class NNRenderBackendType
    {
        Auto,
        Backend_Vulkan,
        Backend_D3D12,
        Backend_D3D11,
        Backend_Metal,
        Backend_OpenGL,
        Backend_WebGPU
    };
}
