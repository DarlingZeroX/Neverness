#pragma once

#include <NNRuntimeRender/Device/INNRenderDevice.h>
#include <NNRuntimeRender/Device/NNDeviceInfo.h>
#include <NNRuntimeCore/NNObject.h>
#include <vector>

namespace NN::Runtime::Render
{
    class NNRenderBootstrap
    {
    public:
        static NN::Runtime::Core::NNRef<INNRenderDevice> CreateDevice(const NNRenderDeviceCreateInfo& info);
        static std::vector<NNRenderBackendType> GetAvailableBackends();
        static const char* GetBackendName(NNRenderBackendType type);
        static NNRenderBackendType DetectBestBackend();
    };

} // namespace NN::Runtime::Render
