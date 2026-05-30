/**
 * @file ViewportRenderRuntimeApi.cpp
 * @brief NNViewportRenderAPI Runtime 实现：将 RenderSceneToTexture 转发至 SceneRenderer。
 *
 * 使用进程级 SceneRenderer 单例，惰性初始化（首次调用时创建）。
 * sceneHandle → SceneSubsystem::GetRuntimeScene() → SceneRenderer::Render()
 */

#include "Internal/RuntimeApiBuilders.h"

#include "NNNativeEngineAPI/Include/NativeInterop.h"
#include "NNNativeEngineAPI/Include/ViewportRenderAPI.h"
#include "NNRuntimeEngine/Include/NNEngineRuntime.h"
#include "Renderer2D/SceneRenderer.h"
#include "NNRuntimeRmlui/Include/Renderer/RmlUIRenderer.h"
#include "NNRuntimeRmlui/Include/System/NNRmlUISystem.h"
#include "NNRuntimeRmlui/Include/System/NNRmlUIModule.h"

#include <iostream>

#include "NNCore/Interface/HLog.h"

namespace
{
using NN::Runtime::engine::NNEngineRuntime;

// 进程级渲染器单例
NN::Runtime::Renderer2D::SceneRenderer* g_SceneRenderer = nullptr;
NN::Runtime::Renderer::RmlUIRenderer* g_RmlUIRenderer = nullptr;
NN::Runtime::RmlUI::NNRmlUISystem* g_RmlUISystem = nullptr;
bool g_Initialized = false;

/// 确保渲染器已初始化（惰性初始化）
bool EnsureSceneRenderer()
{
    if (g_Initialized)
        return g_SceneRenderer != nullptr;

    g_SceneRenderer = new NN::Runtime::Renderer2D::SceneRenderer();
    if (!g_SceneRenderer->Initialize())
    {
        std::cerr << "[ViewportRender] SceneRenderer 初始化失败" << std::endl;
        delete g_SceneRenderer;
        g_SceneRenderer = nullptr;
    }

    // 初始化 RmlUI 渲染器
    g_RmlUIRenderer = new NN::Runtime::Renderer::RmlUIRenderer();
    if (!g_RmlUIRenderer->Initialize(1280, 720))
    {
        std::cerr << "[ViewportRender] RmlUIRenderer 初始化失败" << std::endl;
        delete g_RmlUIRenderer;
        g_RmlUIRenderer = nullptr;
    }

    // 创建 RmlUI 系统（负责构建 DrawList）
    g_RmlUISystem = NN::Runtime::RmlUI::CreateRmlUISystem();

    g_Initialized = true;
    return g_SceneRenderer != nullptr;
}

std::uint64_t NN_ENGINE_ABI_STDCALL rt_viewportRender_renderSceneToTexture(
    std::uint64_t sceneHandle,
    std::uint32_t width,
    std::uint32_t height)
{
    if (!EnsureSceneRenderer())
    {
		H_LOG_WARN("Failed to ensure SceneRenderer initialization");
		return 0;
    }

    // 从句柄获取 NNRuntimeScene
    auto* scene = NNEngineRuntime::Instance().Scene().GetRuntimeScene(sceneHandle);
    if (!scene)
    {
		H_LOG_WARN("Invalid scene handle: {}", sceneHandle);
		return 0;
    }

    // 1. 渲染 Sprite 场景
    std::uint32_t textureId = g_SceneRenderer->Render(*scene, width, height);

    // 2. RmlUI: 构建 DrawList
    if (g_RmlUISystem)
    {
        g_RmlUISystem->Tick(*scene, 0.0f);
    }

    // 3. RmlUI: 同步 + 更新 + 渲染
    if (g_RmlUIRenderer && g_RmlUISystem)
    {
        const auto& drawList = g_RmlUISystem->GetDrawList();
        g_RmlUIRenderer->Sync(drawList);
        g_RmlUIRenderer->Update();
        g_RmlUIRenderer->Render(drawList, NN::Runtime::Scene::NNRmlUIViewTarget::Scene);
    }

    return textureId;
}

std::uint64_t NN_ENGINE_ABI_STDCALL rt_viewportRender_getLastRenderedTexture(void)
{
    if (!g_SceneRenderer)
        return 0;
    return g_SceneRenderer->GetOutputTextureId();
}

void NN_ENGINE_ABI_STDCALL rt_viewportRender_getRenderStats(
    std::uint32_t* outDrawCalls,
    std::uint32_t* outQuadCount)
{
    if (!g_SceneRenderer)
    {
        if (outDrawCalls) *outDrawCalls = 0;
        if (outQuadCount) *outQuadCount = 0;
        return;
    }
    if (outDrawCalls) *outDrawCalls = g_SceneRenderer->GetDrawCallCount();
    if (outQuadCount) *outQuadCount = g_SceneRenderer->GetQuadCount();
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

} // namespace

extern "C" void NNBuildViewportRenderRuntimeApi(NNViewportRenderAPI* api)
{
    std::cout << "Building ViewportRender Runtime API..." << std::endl;
    if (api == nullptr)
    {
        return;
    }

    api->RenderSceneToTexture  = &rt_viewportRender_renderSceneToTexture;
    api->GetLastRenderedTexture = &rt_viewportRender_getLastRenderedTexture;
    api->GetRenderStats        = &rt_viewportRender_getRenderStats;
    api->SetRmlUIViewportSize  = &rt_viewportRender_setRmlUIViewportSize;
    api->ProcessRmlUIInput     = &rt_viewportRender_processRmlUIInput;

    std::cout << "ViewportRender Runtime API built." << std::endl;
}
