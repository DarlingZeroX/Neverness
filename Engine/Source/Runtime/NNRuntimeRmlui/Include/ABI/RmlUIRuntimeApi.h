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

#ifdef __cplusplus
extern "C" {
#endif

/** @brief 填充指向 RmlUI 渲染器的 NNViewportRenderAPI 函数表。 */
NN_RUNTIME_RMLUI_API void NNBuildRmlUIRuntimeApi(NNViewportRenderAPI* api);

#ifdef __cplusplus
} /* extern "C" */
#endif
