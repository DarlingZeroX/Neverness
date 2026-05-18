#pragma once

/**
 * @file RuntimeApiBuilders.h
 * @brief 將 **NNEngineRuntime** 子系統能力覆寫至 **NNNativeEngineAPI** 子表（僅寫函數指標）。
 */

#include "NNNativeEngineAPI/Include/AssetAPI.h"
#include "NNNativeEngineAPI/Include/AssetRegistryAPI.h"
#include "NNNativeEngineAPI/Include/AsyncWaitAPI.h"
#include "NNNativeEngineAPI/Include/EntityAPI.h"
#include "NNNativeEngineAPI/Include/ObjectAPI.h"
#include "NNNativeEngineAPI/Include/SceneAPI.h"
#include "NNNativeEngineAPI/Include/TimingAPI.h"

#ifdef __cplusplus
extern "C" {
#endif

void NNBuildTimingRuntimeApi(NNTimingAPI* api);
void NNBuildAsyncWaitRuntimeApi(NNAsyncWaitAPI* api);
void NNBuildSceneRuntimeApi(NNSceneAPI* api);
void NNBuildAssetRuntimeApi(NNAssetAPI* api);
void NNBuildObjectRuntimeApi(NNObjectAPI* api);
void NNBuildAssetRegistryRuntimeApi(NNAssetRegistryAPI* api);
void NNBuildEntityRuntimeApi(NNEntityAPI* api);

#ifdef __cplusplus
} /* extern "C" */
#endif
