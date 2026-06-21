/**
 * @file ViewportRenderRuntimeApi.cpp
 * @brief NNViewportRenderAPI Runtime 实现：RmlUI 渲染管理。
 *
 * 已移除：
 * - SceneRenderer + NNRuntimeScene 渲染管线（移至 Legacy）
 * - renderSceneToTexture / getLastRenderedTexture / getRenderStats
 *
 * 保留：
 * - RmlUI 渲染器初始化/销毁
 * - RmlUI 输入处理、视口设置
 */

#include "Internal/RuntimeApiBuilders.h"

#include "NNNativeEngineAPI/Include/NativeInterop.h"
#include "NNNativeEngineAPI/Include/ViewportRenderAPI.h"
#include "NNRuntimeRmlui/Include/Renderer/RmlUIRenderer.h"
#include "NNRuntimeRmlui/Include/System/NNRmlUISystem.h"
#include "NNRuntimeRmlui/Include/System/NNRmlUIModule.h"
#include "NNRuntimeRmlui/Include/System/RmlUIAssetResolver.h"
#include "NNRuntimeDiligent/Device/NNDiligentDevice.h"
#include "NativeEngineRuntimeServices.h"

// 获取 Diligent 设备
#include "Core/WindowRegistry.h"
#include <Device/INNRenderDevice.h>

#include <iostream>
#include <exception>

#include "NNCore/Interface/HLog.h"

namespace
{
/**
 * @brief 进程级 RmlUI 渲染器单例管理。
 */
NN::Runtime::Renderer::RmlUIRenderer* g_RmlUIRenderer = nullptr;
NN::Runtime::RmlUI::NNRmlUISystem* g_RmlUISystem = nullptr;
bool g_Initialized = false;
std::uint64_t g_LastRmluiTextureId = 0;

/// 确保 RmlUI 渲染器已初始化（惰性初始化）
bool EnsureRmlUIRenderer()
{
    if (g_Initialized)
        return g_RmlUIRenderer != nullptr;

    std::cout << "[ViewportRender] EnsureRmlUIRenderer: 开始初始化" << std::endl;

    // 从主窗口获取 Diligent 设备
    NN::Runtime::Render::INNRenderDevice* device = nullptr;
    if (auto* window = NN::Runtime::WindowRegistry::Resolve(NN::Runtime::WindowRegistry::GetPrimaryHandle()))
    {
        device = window->GetDevice();
    }
    if (!device)
    {
        std::cerr << "[ViewportRender] 无法获取 Diligent 设备" << std::endl;
        return false;
    }

    // 初始化 RmlUI 渲染器
    std::cout << "[ViewportRender] 初始化 RmlUIRenderer..." << std::endl;
    g_RmlUIRenderer = new NN::Runtime::Renderer::RmlUIRenderer();
    if (!g_RmlUIRenderer->Initialize(device, 1280, 720))
    {
        std::cerr << "[ViewportRender] RmlUIRenderer 初始化失败" << std::endl;
        delete g_RmlUIRenderer;
        g_RmlUIRenderer = nullptr;
    }
    else
    {
        std::cout << "[ViewportRender] RmlUIRenderer 初始化成功" << std::endl;
    }

    // 创建 RmlUI 系统（负责构建 DrawList）
    g_RmlUISystem = NN::Runtime::RmlUI::CreateRmlUISystem();
    std::cout << "[ViewportRender] RmlUISystem 已创建" << std::endl;

    g_Initialized = true;
    std::cout << "[ViewportRender] EnsureRmlUIRenderer: 初始化完成" << std::endl;
    return g_RmlUIRenderer != nullptr;
}

void NN_ENGINE_ABI_STDCALL rt_viewportRender_setRmlUIViewportSize(
    std::uint32_t width,
    std::uint32_t height)
{
    if (g_RmlUIRenderer)
        g_RmlUIRenderer->SetViewport(width, height);
}

void NN_ENGINE_ABI_STDCALL rt_viewportRender_processRmlUIInput(
    std::uint32_t type,
    std::int32_t mouseX, std::int32_t mouseY,
    std::int32_t wheelX, std::int32_t wheelY,
    std::uint32_t button,
    std::uint32_t keyCode, std::uint32_t keyMod)
{
    if (g_RmlUIRenderer)
        g_RmlUIRenderer->ProcessInput(type, mouseX, mouseY, wheelX, wheelY, button, keyCode, keyMod);
}

std::uint64_t NN_ENGINE_ABI_STDCALL rt_viewportRender_getLastRmluiTexture(void)
{
    return g_LastRmluiTextureId;
}

} // namespace

// ── Getter 函数（供 ViewportSurfaceRuntimeApi 访问 RmlUI 单例） ──

/// 获取 RmlUI 渲染器单例（由 ViewportRenderRuntimeApi 管理生命周期）。
NN::Runtime::Renderer::RmlUIRenderer* GetRmlUIRenderer()
{
    return g_RmlUIRenderer;
}

/// 获取 RmlUI 系统单例（由 ViewportRenderRuntimeApi 管理生命周期）。
NN::Runtime::RmlUI::NNRmlUISystem* GetRmlUISystem()
{
    return g_RmlUISystem;
}

/// 确保 ViewportRender 的 RmlUI 单例已初始化（惰性初始化，可重复调用）。
void EnsureViewportRenderInitialized()
{
    EnsureRmlUIRenderer();
}

/// 关闭 ViewportRender 资源（引擎退出时调用）
void ShutdownViewportRender()
{
    if (g_RmlUIRenderer)
    {
        g_RmlUIRenderer->Shutdown();
        delete g_RmlUIRenderer;
        g_RmlUIRenderer = nullptr;
    }
    if (g_RmlUISystem)
    {
        NN::Runtime::RmlUI::DestroyRmlUISystem(g_RmlUISystem);
        g_RmlUISystem = nullptr;
    }
    g_Initialized = false;
    g_LastRmluiTextureId = 0;
}

extern "C" void NNBuildViewportRenderRuntimeApi(NNViewportRenderAPI* api)
{
    std::cout << "Building ViewportRender Runtime API..." << std::endl;
    if (api == nullptr)
    {
        return;
    }

    api->SetRmlUIViewportSize   = &rt_viewportRender_setRmlUIViewportSize;
    api->ProcessRmlUIInput      = &rt_viewportRender_processRmlUIInput;
    api->GetLastRmluiTexture    = &rt_viewportRender_getLastRmluiTexture;

    std::cout << "ViewportRender Runtime API built." << std::endl;
}
