#pragma once

/**
 * @file ApiStubBuilders.h
 * @brief 各 Engine Service 子表之 Stub 建表函式宣告（僅寫入函數指標，不修改 **layoutVersion**）。
 */

#include "ApplicationAPI.h"
#include "AssetAPI.h"
#include "AssetRegistryAPI.h"
#include "AsyncWaitAPI.h"
#include "AudioAPI.h"
#include "EntityAPI.h"
#include "InputAPI.h"
#include "ObjectAPI.h"
#include "RenderAPI.h"
#include "SceneAPI.h"
#include "TimingAPI.h"
#include "UIAPI.h"

#ifdef __cplusplus
extern "C" {
#endif

void NNBuildApplicationApiStubs(NNApplicationAPI* api);
void NNBuildRenderApiStubs(NNRenderAPI* api);
void NNBuildUiApiStubs(NNUIAPI* api);
void NNBuildAudioApiStubs(NNAudioAPI* api);
void NNBuildAssetApiStubs(NNAssetAPI* api);
void NNBuildInputApiStubs(NNInputAPI* api);
void NNBuildSceneApiStubs(NNSceneAPI* api);
void NNBuildTimingApiStubs(NNTimingAPI* api);
void NNBuildAsyncWaitApiStubs(NNAsyncWaitAPI* api);
void NNBuildObjectApiStubs(NNObjectAPI* api);
void NNBuildAssetRegistryApiStubs(NNAssetRegistryAPI* api);
void NNBuildEntityApiStubs(NNEntityAPI* api);

#ifdef __cplusplus
} /* extern "C" */
#endif
