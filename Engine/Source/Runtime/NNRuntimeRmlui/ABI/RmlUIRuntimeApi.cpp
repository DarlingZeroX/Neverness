/**
 * @file RmlUIRuntimeApi.cpp
 * @brief NNViewportRenderAPI Runtime 实现：RmlUI 渲染管理。
 *
 * 从 NNRuntimeEngineServices/ViewportRenderRuntimeApi.cpp 迁入。
 * 管理 RmlUI 渲染器单例的生命周期，填充 NNViewportRenderAPI 函数表。
 *
 * 设备获取：通过 EnsureRmlUIInitialized(device) 参数注入，
 * 由 EngineServices 的 ViewportSurfaceRuntimeApi 在首帧渲染时传入。
 *
 * 导出：
 *   - NNBuildRmlUIRuntimeApi()  — 填充 NNViewportRenderAPI 函数指针
 *   - ShutdownRmlUI()           — 引擎退出时清理 RmlUI 资源
 *   - GetRmlUIRenderer()        — 获取渲染器单例（供 ViewportSurfaceRuntimeApi 使用）
 *   - GetRmlUISystem()          — 获取系统单例
 *   - EnsureRmlUIInitialized()  — 惰性初始化（接收设备指针）
 */

#include "ABI/RmlUIRuntimeApi.h"

#include "Engine/ViewportRenderAPI.h"
#include "NNRuntimeRmlui/Include/Renderer/RmlUIRenderer.h"
#include "NNRuntimeRmlui/Include/System/NNRmlUISystem.h"
#include "NNRuntimeRmlui/Include/System/NNRmlUIModule.h"

#include <Device/INNRenderDevice.h>

#include <iostream>

namespace
{
/**
 * @brief 进程级 RmlUI 渲染器单例管理。
 */
NN::Runtime::Renderer::RmlUIRenderer* g_RmlUIRenderer = nullptr;
NN::Runtime::RmlUI::NNRmlUISystem* g_RmlUISystem = nullptr;
NN::Runtime::Render::INNRenderDevice* g_RenderDevice = nullptr;
bool g_Initialized = false;
std::uint64_t g_LastRmluiTextureId = 0;

/// 确保 RmlUI 渲染器已初始化（惰性初始化）
bool EnsureRmlUIRenderer(NN::Runtime::Render::INNRenderDevice* device)
{
    if (g_Initialized)
        return g_RmlUIRenderer != nullptr;

    // 首次调用时缓存设备指针
    if (device)
        g_RenderDevice = device;

    if (!g_RenderDevice)
    {
        std::cerr << "[RmlUIRuntimeApi] 渲染设备为空，无法初始化" << std::endl;
        g_Initialized = true;
        return false;
    }

    std::cout << "[RmlUIRuntimeApi] EnsureRmlUIRenderer: 开始初始化" << std::endl;

    // 初始化 RmlUI 渲染器
    g_RmlUIRenderer = new NN::Runtime::Renderer::RmlUIRenderer();
    if (!g_RmlUIRenderer->Initialize(g_RenderDevice, 1280, 720))
    {
        std::cerr << "[RmlUIRuntimeApi] RmlUIRenderer 初始化失败" << std::endl;
        delete g_RmlUIRenderer;
        g_RmlUIRenderer = nullptr;
    }
    else
    {
        std::cout << "[RmlUIRuntimeApi] RmlUIRenderer 初始化成功" << std::endl;
    }

    // 创建 RmlUI 系统（负责构建 DrawList）
    g_RmlUISystem = NN::Runtime::RmlUI::CreateRmlUISystem();
    std::cout << "[RmlUIRuntimeApi] RmlUISystem 已创建" << std::endl;

    g_Initialized = true;
    return g_RmlUIRenderer != nullptr;
}

// ── NNViewportRenderAPI 函数实现 ──

void NN_ENGINE_ABI_STDCALL rt_rmlUI_setViewportSize(
    std::uint32_t width,
    std::uint32_t height)
{
    if (g_RmlUIRenderer)
        g_RmlUIRenderer->SetViewport(width, height);
}

void NN_ENGINE_ABI_STDCALL rt_rmlUI_processInput(
    std::uint32_t type,
    std::int32_t mouseX, std::int32_t mouseY,
    std::int32_t wheelX, std::int32_t wheelY,
    std::uint32_t button,
    std::uint32_t keyCode, std::uint32_t keyMod)
{
    if (g_RmlUIRenderer)
        g_RmlUIRenderer->ProcessInput(type, mouseX, mouseY, wheelX, wheelY, button, keyCode, keyMod);
}

std::uint64_t NN_ENGINE_ABI_STDCALL rt_rmlUI_getLastTexture(void)
{
    return g_LastRmluiTextureId;
}

} // namespace

// ── Getter / 初始化函数 ──

NN::Runtime::Renderer::RmlUIRenderer* GetRmlUIRenderer()
{
    return g_RmlUIRenderer;
}

NN::Runtime::RmlUI::NNRmlUISystem* GetRmlUISystem()
{
    return g_RmlUISystem;
}

void EnsureRmlUIInitialized(NN::Runtime::Render::INNRenderDevice* device)
{
    EnsureRmlUIRenderer(device);
}

// ── 导出函数 ──

extern "C" void NNBuildRmlUIRuntimeApi(NNViewportRenderAPI* api)
{
    std::cout << "[RmlUIRuntimeApi] Building RmlUI Runtime API..." << std::endl;
    if (api == nullptr)
        return;

    api->SetRmlUIViewportSize   = &rt_rmlUI_setViewportSize;
    api->ProcessRmlUIInput      = &rt_rmlUI_processInput;
    api->GetLastRmluiTexture    = &rt_rmlUI_getLastTexture;
    // 热重载 API — 待 RmlUIRenderer 实现 ReloadDocument 后接线
    api->ReloadRmlDocument      = nullptr;
    api->ReloadAllRmlDocuments  = nullptr;

    std::cout << "[RmlUIRuntimeApi] RmlUI Runtime API built." << std::endl;
}

extern "C" void ShutdownRmlUI(void)
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
    g_RenderDevice = nullptr;
}
