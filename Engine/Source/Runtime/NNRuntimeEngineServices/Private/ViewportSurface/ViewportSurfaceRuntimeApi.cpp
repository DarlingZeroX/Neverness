/**
 * @file ViewportSurfaceRuntimeApi.cpp
 * @brief NNViewportSurfaceAPI Runtime 实现：管理多个 ViewportSurface 的 SwapChain。
 *
 * 设计：
 * - 每个 Surface 持有独立 SwapChain（复用主窗口的 Device/Context）
 * - Deferred Resize：MarkResize → FlushResizes（帧末统一执行）
 * - Surface Lost：HandleDestroyed → MarkLost → HandleCreated → Recreate
 * - 进程级 Surface 注册表，monotonic ID 分配
 */

#include "Internal/RuntimeApiBuilders.h"

#include "NNNativeEngineAPI/Include/NativeInterop.h"
#include "NNNativeEngineAPI/Include/ViewportSurfaceAPI.h"
#include "Core/WindowRegistry.h"
#include <Device/INNRenderDevice.h>

// NNRuntimeDiligent（NNDiligentViewportSurface、NNDiligentDevice）
#include "Device/NNDiligentViewportSurface.h"
#include "Device/NNDiligentDevice.h"

#include <iostream>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <vector>

#include "NativeEngineRuntimeServices.h"

namespace
{
    // ── Surface 注册表 ──

    std::mutex g_SurfaceMutex;
    std::atomic<std::uint64_t> g_NextSurfaceId{1};
    std::unordered_map<std::uint64_t, NNDiligent::NNDiligentViewportSurface*> g_Surfaces;

    // 延迟 Resize 条目
    struct ResizeEntry
    {
        std::uint64_t surfaceId;
        std::uint32_t width;
        std::uint32_t height;
    };
    std::vector<ResizeEntry> g_PendingResizes;

    /// 查找 Surface
    NNDiligent::NNDiligentViewportSurface* FindSurface(std::uint64_t surfaceId)
    {
        auto it = g_Surfaces.find(surfaceId);
        return (it != g_Surfaces.end()) ? it->second : nullptr;
    }

    // ── API 实现 ──

    std::uint64_t NN_ENGINE_ABI_STDCALL rt_viewportSurface_createSurface(
        void* nativeHandle,
        NNNativeHandleType handleType,
        std::uint32_t width,
        std::uint32_t height)
    {
        if (!nativeHandle)
        {
            std::cerr << "[ViewportSurface] CreateSurface 参数无效: nativeHandle 为空" << std::endl;
            return 0;
        }

        // 最小尺寸保护（避免创建 0x0 SwapChain）
        if (width < 1) width = 1;
        if (height < 1) height = 1;

        // 获取主窗口的 Diligent Device/Context
        auto primaryHandle = NN::Runtime::WindowRegistry::GetPrimaryHandle();
        if (primaryHandle == 0)
        {
            std::cerr << "[ViewportSurface] 主窗口未创建" << std::endl;
            return 0;
        }

        auto* window = NN::Runtime::WindowRegistry::Resolve(primaryHandle);
        if (!window)
        {
            std::cerr << "[ViewportSurface] 主窗口无法解析" << std::endl;
            return 0;
        }

        // 通过 VGWindow 获取 INNRenderDevice，cast 为 NNDiligentDevice
        // Neverness 目前只使用 NNDiligent 作为渲染后端，此 cast 安全
        auto* iDevice = static_cast<NN::Runtime::Render::INNRenderDevice*>(window->GetDevice());
        if (!iDevice)
        {
            std::cerr << "[ViewportSurface] Diligent 设备未初始化" << std::endl;
            return 0;
        }

        auto* diliDevice = static_cast<NNDiligent::NNDiligentDevice*>(iDevice);
        auto* device  = diliDevice->GetDiligentDevice();
        auto* context = diliDevice->GetDiligentContext();
        if (!device || !context)
        {
            std::cerr << "[ViewportSurface] Diligent Device/Context 无效" << std::endl;
            return 0;
        }

        // 创建 Surface
        auto* surface = new NNDiligent::NNDiligentViewportSurface();
        if (!surface->Create(device, context, nativeHandle, static_cast<uint32_t>(handleType), width, height))
        {
            std::cerr << "[ViewportSurface] SwapChain 创建失败" << std::endl;
            delete surface;
            return 0;
        }

        // 分配 surfaceId 并注册
        std::uint64_t surfaceId = g_NextSurfaceId.fetch_add(1);
        {
            std::lock_guard<std::mutex> lock(g_SurfaceMutex);
            g_Surfaces[surfaceId] = surface;
        }

        std::cout << "[ViewportSurface] Surface " << surfaceId << " 已创建: "
                  << width << "x" << height << std::endl;
        return surfaceId;
    }

    void NN_ENGINE_ABI_STDCALL rt_viewportSurface_destroySurface(std::uint64_t surfaceId)
    {
        std::lock_guard<std::mutex> lock(g_SurfaceMutex);
        auto it = g_Surfaces.find(surfaceId);
        if (it != g_Surfaces.end())
        {
            it->second->Destroy();
            delete it->second;
            g_Surfaces.erase(it);
            std::cout << "[ViewportSurface] Surface " << surfaceId << " 已销毁" << std::endl;
        }
    }

    void NN_ENGINE_ABI_STDCALL rt_viewportSurface_markResize(
        std::uint64_t surfaceId,
        std::uint32_t width,
        std::uint32_t height)
    {
        std::lock_guard<std::mutex> lock(g_SurfaceMutex);
        auto* surface = FindSurface(surfaceId);
        if (surface)
        {
            g_PendingResizes.push_back({surfaceId, width, height});
        }
    }

    void NN_ENGINE_ABI_STDCALL rt_viewportSurface_flushResizes()
    {
        std::lock_guard<std::mutex> lock(g_SurfaceMutex);

        // 合并同一 surface 的多次 resize（只保留最后一次）
        std::unordered_map<std::uint64_t, ResizeEntry> merged;
        for (const auto& entry : g_PendingResizes)
        {
            merged[entry.surfaceId] = entry;
        }
        g_PendingResizes.clear();

        // 执行所有 resize
        for (const auto& [id, entry] : merged)
        {
            auto* surface = FindSurface(id);
            if (surface)
            {
                surface->MarkResize(entry.width, entry.height);
                surface->FlushResize();
            }
        }
    }

    void NN_ENGINE_ABI_STDCALL rt_viewportSurface_present(std::uint64_t surfaceId)
    {
        std::lock_guard<std::mutex> lock(g_SurfaceMutex);
        auto* surface = FindSurface(surfaceId);
        if (surface)
        {
            surface->Present();
        }
    }

    std::uint8_t NN_ENGINE_ABI_STDCALL rt_viewportSurface_isSurfaceLost(std::uint64_t surfaceId)
    {
        std::lock_guard<std::mutex> lock(g_SurfaceMutex);
        auto* surface = FindSurface(surfaceId);
        return surface ? (surface->IsSurfaceLost() ? 1 : 0) : 0;
    }

    std::uint8_t NN_ENGINE_ABI_STDCALL rt_viewportSurface_recreateSurface(
        std::uint64_t surfaceId,
        void* newHandle,
        NNNativeHandleType newHandleType)
    {
        std::lock_guard<std::mutex> lock(g_SurfaceMutex);
        auto* surface = FindSurface(surfaceId);
        if (!surface)
            return 0;

        return surface->Recreate(newHandle, static_cast<uint32_t>(newHandleType)) ? 1 : 0;
    }

    std::uint8_t NN_ENGINE_ABI_STDCALL rt_viewportSurface_renderViewport(
        std::uint64_t surfaceId,
        std::uint64_t sceneHandle,
        std::uint32_t width,
        std::uint32_t height)
    {
        if (surfaceId == 0 || sceneHandle == 0 || width == 0 || height == 0)
            return 0;

        // 0. 获取 SwapChain
        std::lock_guard<std::mutex> lock(g_SurfaceMutex);
        auto* surface = FindSurface(surfaceId);
        if (!surface || !surface->IsCreated())
            return 0;

        auto* swapChain = surface->GetSwapChain();
        if (!swapChain)
            return 0;

        // 1. 渲染场景到离屏 FBO（通过 ViewportRender API）
        //    内部会：SceneRenderer::Render → EndRenderPass → RmlUI::CompositeOnTop → EndRenderPass
        const auto* api = NNNativeEngineApi_GetRuntimeTable();
        if (!api || !api->viewportRender.RenderSceneToTexture)
            return 0;

        std::uint64_t sceneTextureId = api->viewportRender.RenderSceneToTexture(sceneHandle, width, height);
        if (sceneTextureId == 0)
            return 0;

        // 2. 确保 SwapChain 尺寸与渲染目标一致
        auto& swapChainDesc = swapChain->GetDesc();
        if (swapChainDesc.Width != width || swapChainDesc.Height != height)
        {
            swapChain->Resize(width, height);
        }

        // 3. 获取离屏纹理（ITextureView* 编码为 uint64_t）
        auto* srcTextureView = reinterpret_cast<::Diligent::ITextureView*>(sceneTextureId);
        if (!srcTextureView)
            return 0;

        auto* srcTexture = srcTextureView->GetTexture();
        if (!srcTexture)
            return 0;

        // 4. 获取 SwapChain back buffer 纹理
        auto* dstRTV = swapChain->GetCurrentBackBufferRTV();
        if (!dstRTV)
            return 0;

        auto* dstTexture = dstRTV->GetTexture();
        if (!dstTexture)
            return 0;

        // 5. CopyTexture（FBO → SwapChain back buffer）+ Present
        //    CopyTexture 和 Present 在同一个命令列表中，一起提交
        auto* context = surface->GetContext();
        if (!context)
            return 0;

        ::Diligent::CopyTextureAttribs copyAttribs;
        copyAttribs.pSrcTexture = srcTexture;
        copyAttribs.SrcTextureTransitionMode = ::Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
        copyAttribs.pDstTexture = dstTexture;
        copyAttribs.DstTextureTransitionMode = ::Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
        context->CopyTexture(copyAttribs);

        // 6. Present（内部会 Flush + Close 命令列表）
        swapChain->Present();

        return 1;
    }

} // anonymous namespace

// ── Builder ──

extern "C" void NNBuildViewportSurfaceRuntimeApi(NNViewportSurfaceAPI* api)
{
    std::cout << "Building ViewportSurface Runtime API..." << std::endl;
    if (api == nullptr)
        return;

    api->CreateSurface   = &rt_viewportSurface_createSurface;
    api->DestroySurface  = &rt_viewportSurface_destroySurface;
    api->MarkResize      = &rt_viewportSurface_markResize;
    api->FlushResizes    = &rt_viewportSurface_flushResizes;
    api->Present         = &rt_viewportSurface_present;
    api->IsSurfaceLost   = &rt_viewportSurface_isSurfaceLost;
    api->RecreateSurface = &rt_viewportSurface_recreateSurface;
    api->RenderViewport  = &rt_viewportSurface_renderViewport;

    std::cout << "ViewportSurface Runtime API built." << std::endl;
}

// ── Shutdown ──

void ShutdownViewportSurface()
{
    std::lock_guard<std::mutex> lock(g_SurfaceMutex);
    for (auto& [id, surface] : g_Surfaces)
    {
        surface->Destroy();
        delete surface;
    }
    g_Surfaces.clear();
    g_PendingResizes.clear();
    std::cout << "[ViewportSurface] 已关闭" << std::endl;
}
