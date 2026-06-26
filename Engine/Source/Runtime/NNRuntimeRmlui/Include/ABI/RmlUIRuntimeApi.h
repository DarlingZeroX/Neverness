#pragma once

/**
 * @file RmlUIRuntimeApi.h
 * @brief RmlUI Runtime API — 单例 getter + 初始化 + builder + shutdown。
 *
 * 供 EngineServices 的 ViewportSurfaceRuntimeApi 和 NativeEngineRuntimeApiTable 调用。
 * 通过 extern "C" dllexport 导出，与 VFS 模块模式一致。
 */

#include "Engine/ViewportRenderAPI.h"
#include "../../RuntimeRmlUIExport.h"

// 前向声明
namespace NN::Runtime::Renderer { class RmlUIRenderer; }
namespace NN::Runtime::RmlUI { class NNRmlUISystem; }
namespace NN::Runtime::Render { class INNRenderDevice; }

#ifdef __cplusplus
extern "C" {
#endif

/** @brief 填充指向 RmlUI 渲染器的 NNViewportRenderAPI 函数表。 */
NN_RUNTIME_RMLUI_API void NNBuildRmlUIRuntimeApi(NNViewportRenderAPI* api);

/** @brief 关闭 RmlUI 渲染器资源（引擎退出时调用）。 */
NN_RUNTIME_RMLUI_API void ShutdownRmlUI(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

// ── C++ getter / 初始化函数（供 ViewportSurfaceRuntimeApi 内部调用） ──

/** @brief 获取 RmlUI 渲染器单例（由本模块管理生命周期）。 */
NN_RUNTIME_RMLUI_API NN::Runtime::Renderer::RmlUIRenderer* GetRmlUIRenderer();

/** @brief 获取 RmlUI 系统单例（由本模块管理生命周期）。 */
NN_RUNTIME_RMLUI_API NN::Runtime::RmlUI::NNRmlUISystem* GetRmlUISystem();

/**
 * @brief 确保 RmlUI 单例已初始化（惰性初始化，可重复调用）。
 * @param device Diligent 渲染设备指针（首次调用时必须非空，后续调用可忽略）
 */
NN_RUNTIME_RMLUI_API void EnsureRmlUIInitialized(NN::Runtime::Render::INNRenderDevice* device = nullptr);
