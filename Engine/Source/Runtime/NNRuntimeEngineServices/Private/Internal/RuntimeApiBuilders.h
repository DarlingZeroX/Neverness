#pragma once

/**
 * @file RuntimeApiBuilders.h
 * @brief 將 **NNEngineRuntime** 子系統能力覆寫至 **NNNativeEngineAPI** 子表（僅寫函數指標）。
 */

#include "NNNativeEngineAPI/Include/AssetCookerAPI.h"
#include "NNNativeEngineAPI/Include/AsyncWaitAPI.h"
#include "NNNativeEngineAPI/Include/EditorSceneAPI.h"
#include "NNNativeEngineAPI/Include/ObjectAPI.h"
#include "NNNativeEngineAPI/Include/SceneAPI.h"
#include "NNNativeEngineAPI/Include/TimingAPI.h"
#include "NNNativeEngineAPI/Include/RenderAssetAPI.h"
#include "NNNativeEngineAPI/Include/ViewportRenderAPI.h"
#include "NNNativeEngineAPI/Include/ViewportSurfaceAPI.h"

#ifdef __cplusplus
extern "C" {
#endif

void NNBuildTimingRuntimeApi(NNTimingAPI* api);
void NNBuildAsyncWaitRuntimeApi(NNAsyncWaitAPI* api);
void NNBuildSceneRuntimeApi(NNSceneAPI* api);
void NNBuildEditorSceneRuntimeApi(NNEditorSceneAPI* api);
void NNBuildAssetCookerRuntimeApi(NNAssetCookerAPI* api);
void NNBuildRenderAssetRuntimeApi(NNRenderAssetAPI* api);
void NNBuildViewportRenderRuntimeApi(NNViewportRenderAPI* api);
void NNBuildViewportSurfaceRuntimeApi(NNViewportSurfaceAPI* api);

#ifdef __cplusplus
} /* extern "C" */
#endif

// C++ shutdown functions（引擎退出时调用）
void ShutdownViewportRender();
void ShutdownViewportSurface();
