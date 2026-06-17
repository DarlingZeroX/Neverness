/**
 * @file DiligentRuntimeApi.cpp
 * @brief NNDiligentAPI Runtime 实现：暴露底层 Diligent 设备指针。
 *
 * 设计：
 * - 通过 WindowRegistry → VGWindow → NNDiligentDevice 获取 Diligent 指针
 * - 返回 void* 隔离 Diligent 头文件依赖
 * - SwapChain 生命周期由 C# 端管理
 */

#include "Internal/RuntimeApiBuilders.h"

#include "NNNativeEngineAPI/Include/NativeInterop.h"
#include "NNNativeEngineAPI/Include/DiligentAPI.h"
#include "Core/WindowRegistry.h"
#include <Device/INNRenderDevice.h>

// NNRuntimeDiligent（NNDiligentDevice）
#include "Device/NNDiligentDevice.h"

#include <iostream>

#include "NativeEngineRuntimeServices.h"
#include "Device/NNDiligentViewportSurface.h"

namespace
{
    /// 获取主窗口的 NNDiligentDevice（失败返回 nullptr）
    NNDiligent::NNDiligentDevice* GetPrimaryDiligentDevice()
    {
        auto primaryHandle = NN::Runtime::WindowRegistry::GetPrimaryHandle();
        if (primaryHandle == 0)
        {
            std::cerr << "[Diligent] 主窗口未创建" << std::endl;
            return nullptr;
        }

        auto* window = NN::Runtime::WindowRegistry::Resolve(primaryHandle);
        if (!window)
        {
            std::cerr << "[Diligent] 主窗口无法解析" << std::endl;
            return nullptr;
        }

        // 通过 VGWindow 获取 INNRenderDevice，cast 为 NNDiligentDevice
        auto* iDevice = static_cast<NN::Runtime::Render::INNRenderDevice*>(window->GetDevice());
        if (!iDevice)
        {
            std::cerr << "[Diligent] Diligent 设备未初始化" << std::endl;
            return nullptr;
        }

        return static_cast<NNDiligent::NNDiligentDevice*>(iDevice);
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

    std::cout << "Diligent Runtime API built." << std::endl;
}
