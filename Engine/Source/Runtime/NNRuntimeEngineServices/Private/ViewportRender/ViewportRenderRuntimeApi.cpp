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

#include <iostream>

#include "NNCore/Interface/HLog.h"

namespace
{
using NN::Runtime::engine::NNEngineRuntime;

// 进程级 SceneRenderer 单例
NN::Runtime::Renderer2D::SceneRenderer* g_SceneRenderer = nullptr;
bool g_Initialized = false;

/// 确保 SceneRenderer 已初始化（惰性初始化）
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

    return g_SceneRenderer->Render(*scene, width, height);
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

    std::cout << "ViewportRender Runtime API built." << std::endl;
}
