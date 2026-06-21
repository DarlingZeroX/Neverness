#pragma once

/**
 * @file RuntimeApiBuilders.h
 * @brief 將 **NNEngineRuntime** 子系統能力覆寫至 **NNNativeEngineAPI** 子表（僅寫函數指標）。
 */

#include "NNNativeEngineAPI/Include/AsyncWaitAPI.h"
// 已移除：SceneAPI / EditorSceneAPI / ObjectAPI / TimingAPI（已迁移至 C# 或废弃）
// 已移除：RenderAssetAPI（NNRenderAssets 移至 Legacy）
// #include "NNNativeEngineAPI/Include/RenderAssetAPI.h"
#include "NNNativeEngineAPI/Include/ViewportRenderAPI.h"
#include "NNNativeEngineAPI/Include/ViewportSurfaceAPI.h"
#include "NNNativeEngineAPI/Include/DiligentAPI.h"

#ifdef __cplusplus
extern "C" {
#endif

void NNBuildAsyncWaitRuntimeApi(NNAsyncWaitAPI* api);
// 已移除：NNBuildSceneRuntimeApi / NNBuildEditorSceneRuntimeApi（NNRuntimeScene 移至 Legacy）
// void NNBuildSceneRuntimeApi(NNSceneAPI* api);
// void NNBuildEditorSceneRuntimeApi(NNEditorSceneAPI* api);
// 已移除：NNBuildAssetCookerRuntimeApi（已迁移至 C#）
// 已移除：NNBuildRenderAssetRuntimeApi（NNRenderAssets 移至 Legacy）
// void NNBuildRenderAssetRuntimeApi(NNRenderAssetAPI* api);
void NNBuildViewportRenderRuntimeApi(NNViewportRenderAPI* api);
void NNBuildViewportSurfaceRuntimeApi(NNViewportSurfaceAPI* api);
void NNBuildDiligentRuntimeApi(NNDiligentAPI* api);

#ifdef __cplusplus
} /* extern "C" */
#endif

// C++ shutdown functions（引擎退出时调用）
void ShutdownViewportRender();
void ShutdownViewportSurface();
