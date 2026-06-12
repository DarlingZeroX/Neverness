#include "../Include/NNRenderBootstrap.h"
#include <iostream>

#ifdef NN_WITH_DILIGENT
#include <NNRuntimeDiligent/Device/NNDiligentDevice.h>
#endif

namespace NN::Runtime::Render
{
    NN::Runtime::Core::NNRef<INNRenderDevice> NNRenderBootstrap::CreateDevice(const NNRenderDeviceCreateInfo& info)
    {
        auto backend = info.Backend;
        if (backend == NNRenderBackendType::Auto)
            backend = DetectBestBackend();

        std::cout << "[NNRenderBootstrap] Creating device: " << GetBackendName(backend) << std::endl;

        switch (backend)
        {
#ifdef NN_WITH_DILIGENT
            case NNRenderBackendType::Backend_Vulkan:
            case NNRenderBackendType::Backend_D3D12:
            case NNRenderBackendType::Backend_D3D11:
            case NNRenderBackendType::Backend_Metal:
            case NNRenderBackendType::Backend_OpenGL:
            {
                auto* dev = new NNDiligent::NNDiligentDevice();
                if (dev->Initialize(info))
                    return NN::Runtime::Core::NNRef<INNRenderDevice>(dev);
                delete dev;
                return {};
            }
#endif
            default:
                std::cerr << "[NNRenderBootstrap] No suitable backend!" << std::endl;
                return {};
        }
    }

    std::vector<NNRenderBackendType> NNRenderBootstrap::GetAvailableBackends()
    {
        std::vector<NNRenderBackendType> backends;
#ifdef NN_WITH_DILIGENT
        backends.push_back(NNRenderBackendType::Backend_Vulkan);
        backends.push_back(NNRenderBackendType::Backend_D3D12);
        backends.push_back(NNRenderBackendType::Backend_D3D11);
        backends.push_back(NNRenderBackendType::Backend_OpenGL);
#endif
        return backends;
    }

    const char* NNRenderBootstrap::GetBackendName(NNRenderBackendType type)
    {
        if (type == NNRenderBackendType::Auto) return "Auto";
        if (type == NNRenderBackendType::Backend_Vulkan) return "Vulkan";
        if (type == NNRenderBackendType::Backend_D3D12) return "D3D12";
        if (type == NNRenderBackendType::Backend_D3D11) return "D3D11";
        if (type == NNRenderBackendType::Backend_Metal) return "Metal";
        if (type == NNRenderBackendType::Backend_OpenGL) return "OpenGL";
        if (type == NNRenderBackendType::Backend_WebGPU) return "WebGPU";
        return "Unknown";
    }

    NNRenderBackendType NNRenderBootstrap::DetectBestBackend()
    {
        auto available = GetAvailableBackends();
        for (auto pref : { NNRenderBackendType::Backend_Vulkan, NNRenderBackendType::Backend_D3D12,
                           NNRenderBackendType::Backend_D3D11, NNRenderBackendType::Backend_OpenGL })
        {
            for (auto b : available)
                if (b == pref) return b;
        }
        return NNRenderBackendType::Backend_OpenGL;
    }

} // namespace NN::Runtime::Render
