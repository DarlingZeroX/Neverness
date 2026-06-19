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
#include "NNRuntimeScene/Include/Assets/IAssetResolver.h"
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
using NN::Runtime::engine::NNEngineRuntime;

/**
 * @brief 原生 IAssetResolver 实现——桥接 NNAssetRegistry（NNRuntimeAsset 模块）。
 *
 * 通过 NNNativeEngineApi_GetRuntimeTable() 获取 API 函数指针。
 *
 * 生命周期：进程级全局单例，与 g_RmlUIRenderer 同生同灭。
 */
class AssetRegistryResolver final : public NN::Runtime::Scene::IAssetResolver
{
public:
	bool Resolve(NNGuid guid, char* outPath, std::uint32_t outPathSize) noexcept override
	{
		//const auto* api = NNNativeEngineApi_GetRuntimeTable();
		//if (!api || !api->assetRegistry.resolvePathByGuid)
		//{
		//	std::cerr << "[AssetRegistryResolver] API table or resolvePathByGuid is null" << std::endl;
		//	return false;
		//}
		//
		//// 首次调用时打印注册表状态
		//static bool s_logged = false;
		//if (!s_logged)
		//{
		//	auto countFn = api->assetRegistry.getAssetCount;
		//	std::uint32_t assetCount = countFn ? countFn() : 0;
		//	std::cout << "[AssetRegistryResolver] 注册表资产数: " << assetCount << std::endl;
		//	s_logged = true;
		//}
		//
		//// resolvePathByGuid 返回 > 0 表示成功（路径字节数），-1 表示失败
		//int result = api->assetRegistry.resolvePathByGuid(guid, outPath,
		//	static_cast<std::size_t>(outPathSize));
		//if (result <= 0)
		//{
		//	std::cerr << "[AssetRegistryResolver] GUID not found ("
		//		<< guid.high << ":" << guid.low << ")" << std::endl;
		//}
		return false;
	}
};

// 进程级渲染器单例
NN::Runtime::Renderer2D::SceneRenderer* g_SceneRenderer = nullptr;
NN::Runtime::Renderer::RmlUIRenderer* g_RmlUIRenderer = nullptr;
NN::Runtime::RmlUI::NNRmlUISystem* g_RmlUISystem = nullptr;
AssetRegistryResolver* g_AssetResolver = nullptr;
bool g_Initialized = false;
std::uint64_t g_LastRmluiTextureId = 0;  // RmlUI 渲染结果纹理句柄（Diligent ITextureView*）

/// 确保渲染器已初始化（惰性初始化）
bool EnsureSceneRenderer()
{
    if (g_Initialized)
        return g_SceneRenderer != nullptr;

    std::cout << "[ViewportRender] EnsureSceneRenderer: 开始初始化" << std::endl;

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

    g_SceneRenderer = new NN::Runtime::Renderer2D::SceneRenderer();
    if (!g_SceneRenderer->Initialize(device))
    {
        std::cerr << "[ViewportRender] SceneRenderer 初始化失败" << std::endl;
        delete g_SceneRenderer;
        g_SceneRenderer = nullptr;
    }
    else
    {
        std::cout << "[ViewportRender] SceneRenderer 初始化成功" << std::endl;
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

    // 设置资产路径解析器（桥接 NNAssetRegistry → IAssetResolver）
    g_AssetResolver = new AssetRegistryResolver();
    if (g_RmlUIRenderer)
    {
        g_RmlUIRenderer->SetAssetResolver(g_AssetResolver);
        std::cout << "[ViewportRender] AssetResolver 已设置" << std::endl;
    }

    // 创建 RmlUI 系统（负责构建 DrawList）
    g_RmlUISystem = NN::Runtime::RmlUI::CreateRmlUISystem();
    std::cout << "[ViewportRender] RmlUISystem 已创建" << std::endl;

    g_Initialized = true;
    std::cout << "[ViewportRender] EnsureSceneRenderer: 初始化完成" << std::endl;
    return g_SceneRenderer != nullptr;
}

std::uint64_t NN_ENGINE_ABI_STDCALL rt_viewportRender_renderSceneToTexture(
    std::uint64_t sceneHandle,
    std::uint32_t width,
    std::uint32_t height)
{


    //try
    //{
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
		//H_LOG_INFO("Rendering scene to texture 1");
		std::uint64_t textureId = g_SceneRenderer->Render(*scene, width, height);
		//H_LOG_INFO("Rendering scene to texture 2");
		//return textureId;
	    static int s_renderLogCount = 0;
	    if (s_renderLogCount < 5)
	    {
	        H_LOG_INFO("[ViewportRender] Render: sceneHandle=%llu w=%u h=%u textureId=%llu",
	            sceneHandle, width, height, textureId);
	        s_renderLogCount++;
	    }
	 
	    // 2. RmlUI: 构建 DrawList
	    if (g_RmlUISystem)
	    {
	        g_RmlUISystem->Tick(*scene, 0.0f);
	    }
	 
	    // 3. RmlUI: 在 Scene RT 上叠加渲染（alpha 混合，不创建中间纹理）
	    g_LastRmluiTextureId = 0;
        if (g_RmlUIRenderer && g_RmlUISystem)
        {
            g_RmlUIRenderer->SetViewport(width, height);

            const auto& drawList = g_RmlUISystem->GetDrawList();
            g_RmlUIRenderer->Sync(drawList);

            // 获取 Scene RT，在上面叠加 RmlUI（alpha 混合）
            auto* sceneRT = g_SceneRenderer->GetFramebufferObject();
            if (sceneRT && sceneRT->GetColorRTV())
            {
                g_RmlUIRenderer->RenderOverlayOnScene(
                    drawList, NN::Runtime::Scene::NNRmlUIViewTarget::Scene,
                    sceneRT->GetColorRTV(), sceneRT->GetDepthDSV(), width, height);
            }
        }

        // 返回合成后的 Scene 纹理（Scene + RmlUI 叠加）
        return textureId;
    //}
    //catch (const std::exception& e)
    //{
    //    H_LOG_WARN("[ViewportRender] renderSceneToTexture 异常: {}", e.what());
    //    return 0;
    //}
    //catch (...)
    //{
    //    H_LOG_WARN("[ViewportRender] renderSceneToTexture SEH 异常");
    //    return 0;
    //}
}

std::uint64_t NN_ENGINE_ABI_STDCALL rt_viewportRender_getLastRenderedTexture(void)
{
    if (!g_SceneRenderer)
        return 0;
    return g_SceneRenderer->GetOutputTextureHandle();
}

std::uint64_t NN_ENGINE_ABI_STDCALL rt_viewportRender_getLastRmluiTexture(void)
{
    return g_LastRmluiTextureId;
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

// ── Getter 函数（供 ViewportSurfaceRuntimeApi 访问 RmlUI 单例） ──
// 注意：g_RmlUIRenderer / g_RmlUISystem 在匿名 namespace 中定义（内部链接），
// getter 函数在同一翻译单元中，可以访问它们。

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
    EnsureSceneRenderer();
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
    if (g_SceneRenderer)
    {
        g_SceneRenderer->Shutdown();
        delete g_SceneRenderer;
        g_SceneRenderer = nullptr;
    }
    delete g_AssetResolver;
    g_AssetResolver = nullptr;
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

    api->RenderSceneToTexture   = &rt_viewportRender_renderSceneToTexture;
    api->GetLastRenderedTexture = &rt_viewportRender_getLastRenderedTexture;
    api->GetRenderStats         = &rt_viewportRender_getRenderStats;
    api->SetRmlUIViewportSize   = &rt_viewportRender_setRmlUIViewportSize;
    api->ProcessRmlUIInput      = &rt_viewportRender_processRmlUIInput;
    api->GetLastRmluiTexture    = &rt_viewportRender_getLastRmluiTexture;

    std::cout << "ViewportRender Runtime API built." << std::endl;
}
