#pragma once

/**
 * @file RuntimeApiBuilders.h
 * @brief 將 **NNEngineRuntime** 子系統能力覆寫至 **NNNativeEngineAPI** 子表（僅寫函數指標）。
 */

#include "Engine/AsyncWaitAPI.h"
// 已移除：SceneAPI / EditorSceneAPI / ObjectAPI / TimingAPI（已迁移至 C# 或废弃）
// 已移除：RenderAssetAPI（NNRenderAssets 移至 Legacy）
// #include "Engine/RenderAssetAPI.h"
#include "Engine/ViewportSurfaceAPI.h"
#include "Engine/DiligentAPI.h"

// ViewportRender 已迁移至 NNRuntimeRmlui 模块（ABI/RmlUIRuntimeApi.cpp）
#include "NNRuntimeRmlui/Include/ABI/RmlUIRuntimeApi.h"

#ifdef __cplusplus
extern "C" {
#endif

void NNBuildAsyncWaitRuntimeApi(NNAsyncWaitAPI* api);
void NNBuildViewportSurfaceRuntimeApi(NNViewportSurfaceAPI* api);
void NNBuildDiligentRuntimeApi(NNDiligentAPI* api);

#ifdef __cplusplus
} /* extern "C" */
#endif

// C++ shutdown functions（引擎退出时调用）
void ShutdownViewportSurface();
