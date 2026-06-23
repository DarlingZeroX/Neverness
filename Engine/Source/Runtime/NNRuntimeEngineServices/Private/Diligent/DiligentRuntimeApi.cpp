/**
 * @file DiligentRuntimeApi.cpp
 * @brief NNDiligentAPI Runtime 实现：暴露底层 Diligent 设备指针。
 *
 * 设计：
 * - 全局 NNDiligentDevice 由 C# 端通过 CreateDeviceForNativeHandle 创建
 * - 返回 void* 隔离 Diligent 头文件依赖
 * - SwapChain 生命周期由 C# 端管理
 */

#include "Internal/RuntimeApiBuilders.h"

#include "Engine/NativeInterop.h"
#include "Engine/DiligentAPI.h"
#include <Device/INNRenderDevice.h>

// NNRuntimeDiligent（NNDiligentDevice）
#include "Device/NNDiligentDevice.h"
#include "NNRenderBootstrap.h"

#include <iostream>

#include "NativeEngineRuntimeServices.h"
#include "Device/NNDiligentViewportSurface.h"

namespace
{
    /// 全局 Diligent 设备（C# 端通过 CreateDeviceForNativeHandle 创建）
    NNDiligent::NNDiligentDevice* g_PrimaryDevice = nullptr;

    /// 获取主设备（失败返回 nullptr）
    NNDiligent::NNDiligentDevice* GetPrimaryDiligentDevice()
    {
        if (!g_PrimaryDevice)
        {
            std::cerr << "[Diligent] 设备未创建（需先调用 CreateDeviceForNativeHandle）" << std::endl;
        }
        return g_PrimaryDevice;
    }

    // ── API 实现 ──

    void* NN_ENGINE_ABI_STDCALL rt_diligent_getPrimaryDevice()
    {
        auto* diliDevice = GetPrimaryDiligentDevice();
        if (!diliDevice)
            return nullptr;

        return diliDevice->GetDiligentDevice();
    }

    void* NN_ENGINE_ABI_STDCALL rt_diligent_getPrimaryContext()
    {
        auto* diliDevice = GetPrimaryDiligentDevice();
        if (!diliDevice)
            return nullptr;

        return diliDevice->GetDiligentContext();
    }

    void* NN_ENGINE_ABI_STDCALL rt_diligent_getPrimarySwapChain()
    {
        auto* diliDevice = GetPrimaryDiligentDevice();
        if (!diliDevice)
            return nullptr;

        return diliDevice->GetDiligentSwapChain();
    }

    void* NN_ENGINE_ABI_STDCALL rt_diligent_createViewportSurfaceWithSwapChain(
        void* nativeHandle,
        std::uint32_t handleType,
        std::uint32_t width,
        std::uint32_t height)
    {
        if (!nativeHandle)
        {
            std::cerr << "[Diligent] CreateViewportSurfaceWithSwapChain 参数无效: nativeHandle 为空" << std::endl;
            return nullptr;
        }

        // 最小尺寸保护
        if (width < 1) width = 1;
        if (height < 1) height = 1;

        // 获取 Diligent Device/Context
        auto* diliDevice = GetPrimaryDiligentDevice();
        if (!diliDevice)
            return nullptr;

        auto* device  = diliDevice->GetDiligentDevice();
        auto* context = diliDevice->GetDiligentContext();
        if (!device || !context)
        {
            std::cerr << "[Diligent] Diligent Device/Context 无效" << std::endl;
            return nullptr;
        }

        // 创建 Surface
        auto* surface = new NNDiligent::NNDiligentViewportSurface();
        if (!surface->Create(device, context, nativeHandle, handleType, width, height))
        {
            std::cerr << "[Diligent] SwapChain 创建失败" << std::endl;
            delete surface;
            return nullptr;
        }

        // 返回 SwapChain 指针
        // 注意：Surface 未注册到 ViewportSurfaceAPI 注册表，生命周期由 C# 端管理
        auto* swapChain = surface->GetSwapChain();
        if (!swapChain)
        {
            std::cerr << "[Diligent] SwapChain 指针无效" << std::endl;
            surface->Destroy();
            delete surface;
            return nullptr;
        }

        std::cout << "[Diligent] ViewportSurface 已创建: " << width << "x" << height << std::endl;
        return swapChain;
    }

    std::uint32_t NN_ENGINE_ABI_STDCALL rt_diligent_createDeviceForWindow(
        void* sdlWindow,
        std::uint32_t width,
        std::uint32_t height)
    {
        if (!sdlWindow)
        {
            std::cerr << "[Diligent] CreateDeviceForWindow: sdlWindow 为空" << std::endl;
            return 0;
        }

        // 已有设备时直接返回成功（不重复创建）
        if (g_PrimaryDevice)
        {
            std::cout << "[Diligent] CreateDeviceForWindow: 设备已存在，跳过" << std::endl;
            return 1;
        }

        // 最小尺寸保护
        if (width < 1) width = 1;
        if (height < 1) height = 1;

        // 构建设备创建信息
        NN::Runtime::Render::NNRenderDeviceCreateInfo createInfo{};
        createInfo.Window = sdlWindow;
        createInfo.Width = width;
        createInfo.Height = height;
        createInfo.Backend = NN::Runtime::Render::NNRenderBackendType::Backend_D3D12;
        createInfo.EnableValidation = true;
        createInfo.VSync = true;

        auto device = NN::Runtime::Render::NNRenderBootstrap::CreateDevice(createInfo);
        if (!device)
        {
            std::cerr << "[Diligent] CreateDeviceForWindow: 设备创建失败" << std::endl;
            return 0;
        }

        g_PrimaryDevice = static_cast<NNDiligent::NNDiligentDevice*>(device.Detach());
        std::cout << "[Diligent] CreateDeviceForWindow: 设备创建成功" << std::endl;
        return 1;
    }

    /// 释放全局 Diligent 设备（由 ApplicationShutdown 调用）
    void ReleasePrimaryDevice()
    {
        if (g_PrimaryDevice)
        {
            g_PrimaryDevice->Release();
            g_PrimaryDevice = nullptr;
            std::cout << "[Diligent] 全局设备已释放" << std::endl;
        }
    }

    /// 从平台原生句柄创建 Diligent 设备（绕过 SDL，解决 C#/C++ SDL3 实例不共享问题）
    std::uint32_t NN_ENGINE_ABI_STDCALL rt_diligent_createDeviceForNativeHandle(
        void* nativeHandle,
        std::uint32_t handleType,
        std::uint32_t width,
        std::uint32_t height)
    {
        if (!nativeHandle)
        {
            std::cerr << "[Diligent] CreateDeviceForNativeHandle: nativeHandle 为空" << std::endl;
            return 0;
        }

        // 已有设备时直接返回成功（不重复创建）
        if (g_PrimaryDevice)
        {
            std::cout << "[Diligent] CreateDeviceForNativeHandle: 设备已存在，跳过" << std::endl;
            return 1;
        }

        // 最小尺寸保护
        if (width < 1) width = 1;
        if (height < 1) height = 1;

        auto* device = new NNDiligent::NNDiligentDevice();
        if (!device->InitializeFromNativeHandle(
                nativeHandle, handleType, width, height,
                NN::Runtime::Render::NNRenderBackendType::Backend_D3D12,
                true, true))
        {
            std::cerr << "[Diligent] CreateDeviceForNativeHandle: 设备创建失败" << std::endl;
            device->Release();
            return 0;
        }

        g_PrimaryDevice = device;
        std::cout << "[Diligent] CreateDeviceForNativeHandle: 设备创建成功" << std::endl;
        return 1;
    }

    void NN_ENGINE_ABI_STDCALL rt_diligent_presentPrimarySwapChain()
    {
        auto* diliDevice = GetPrimaryDiligentDevice();
        if (!diliDevice)
            return;

        auto* sc = diliDevice->GetDiligentSwapChain();
        if (sc)
            sc->Present();
    }

    void* NN_ENGINE_ABI_STDCALL rt_diligent_getPrimaryRenderDevice()
    {
        return GetPrimaryDiligentDevice();
    }

} // anonymous namespace

// ── Builder ──

extern "C" void NNBuildDiligentRuntimeApi(NNDiligentAPI* api)
{
    std::cout << "Building Diligent Runtime API..." << std::endl;
    if (api == nullptr)
        return;

    api->GetPrimaryDevice   = &rt_diligent_getPrimaryDevice;
    api->GetPrimaryContext  = &rt_diligent_getPrimaryContext;
    api->GetPrimarySwapChain = &rt_diligent_getPrimarySwapChain;
    api->CreateViewportSurfaceWithSwapChain = &rt_diligent_createViewportSurfaceWithSwapChain;
    api->CreateDeviceForWindow = &rt_diligent_createDeviceForWindow;
    api->CreateDeviceForNativeHandle = &rt_diligent_createDeviceForNativeHandle;
    api->PresentPrimarySwapChain = &rt_diligent_presentPrimarySwapChain;
    api->GetPrimaryRenderDevice = &rt_diligent_getPrimaryRenderDevice;

    std::cout << "Diligent Runtime API built." << std::endl;
}
