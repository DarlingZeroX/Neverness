// NNEngineContext.h -- Singleton engine context: device + handle registry
// Used by NNNativeEngineAPI C functions to access the render device and register resources.

#pragma once

#include <NNRuntimeRender/Device/INNRenderDevice.h>
#include <NNRuntimeCore/Handle/NNObjectHandleRegistry.h>

namespace NN::Runtime::NativeAPI
{
    using namespace NN::Runtime::Core;
    using namespace NN::Runtime::Render;

    // Singleton engine context that holds device + handle registry
    class NNEngineContext
    {
    public:
        static NNEngineContext& Get();

        bool Initialize(INNRenderDevice* device);
        void Shutdown();

        INNRenderDevice* GetDevice() { return m_Device.Get(); }
        NNObjectHandleRegistry& GetRegistry() { return m_Registry; }

        // Register a resource and return handle
        NNRenderHandle RegisterResource(NNHandleType type, INNObject* obj);
        INNObject* GetResource(NNRenderHandle handle);
        void ReleaseResource(NNRenderHandle handle);

    private:
        NNEngineContext() = default;
        NNRef<INNRenderDevice> m_Device;
        NNObjectHandleRegistry m_Registry;
    };

} // namespace NN::Runtime::NativeAPI
