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

#include <iostream>
#include <vector>
#include <string>

// ── 导出函数 ──

extern "C" void NNBuildRmlUIRuntimeApi(NNViewportRenderAPI* api)
{
    std::cout << "[RmlUIRuntimeApi] Building RmlUI Runtime API..." << std::endl;
    if (api == nullptr)
        return;

    //api->SetRmlUIViewportSize   = &rt_rmlUI_setViewportSize;
    //api->ProcessRmlUIInput      = &rt_rmlUI_processInput;
    //api->GetLastRmluiTexture    = &rt_rmlUI_getLastTexture;
    //api->ReloadRmlDocument      = &rt_rmlUI_reloadDocument;
    //api->ReloadAllRmlDocuments  = &rt_rmlUI_reloadAllDocuments;

    std::cout << "[RmlUIRuntimeApi] RmlUI Runtime API built." << std::endl;
}
