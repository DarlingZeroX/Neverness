// NNEngineContext.cpp -- Singleton engine context implementation

#include "../API/NNEngineContext.h"
#include <iostream>

namespace NN::Runtime::NativeAPI
{
    // Singleton static instance
    NNEngineContext& NNEngineContext::Get()
    {
        static NNEngineContext s_Instance;
        return s_Instance;
    }

    bool NNEngineContext::Initialize(INNRenderDevice* device)
    {
        if (!device) return false;
        m_Device = NNRef<INNRenderDevice>(device);
        std::cout << "[NNEngineContext] Initialized with device: "
                  << device->GetDeviceInfo().DeviceName << std::endl;
        return true;
    }

    void NNEngineContext::Shutdown()
    {
        m_Registry.Clear();
        m_Device = nullptr;
        std::cout << "[NNEngineContext] Shutdown" << std::endl;
    }

    NNRenderHandle NNEngineContext::RegisterResource(NNHandleType type, INNObject* obj)
    {
        if (!obj) return NN_INVALID_HANDLE;
        return m_Registry.Register(type, NNRef<INNObject>(obj));
    }

    INNObject* NNEngineContext::GetResource(NNRenderHandle handle)
    {
        return m_Registry.Get(handle);
    }

    void NNEngineContext::ReleaseResource(NNRenderHandle handle)
    {
        m_Registry.Release(handle);
    }

} // namespace NN::Runtime::NativeAPI
