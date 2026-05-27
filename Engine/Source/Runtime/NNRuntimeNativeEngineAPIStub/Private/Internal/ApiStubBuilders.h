#pragma once

/**
 * @file ApiStubBuilders.h
 * @brief 各 Engine Service 子表之 Stub 建表函式宣告（僅寫入函數指標，不修改 **layoutVersion**）。
 */

#include "ApplicationAPI.h"
#include "AssetAPI.h"
#include "AssetManagerAPI.h"
#include "AssetRegistryAPI.h"
#include "AssetCookerAPI.h"
#include "AsyncWaitAPI.h"
#include "AudioAPI.h"
#include "EditorSceneAPI.h"
#include "EntityAPI.h"
#include "InputAPI.h"
#include "ObjectAPI.h"
#include "RenderAPI.h"
#include "SceneAPI.h"
#include "TimingAPI.h"
#include "UIAPI.h"
#include "VfsAPI.h"
#include "WindowAPI.h"
#include "EventAPI.h"
#include "RenderAssetAPI.h"
#include "ViewportRenderAPI.h"

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
void NNBuildEditorSceneApiStubs(NNEditorSceneAPI* api);
void NNBuildTimingApiStubs(NNTimingAPI* api);
void NNBuildAsyncWaitApiStubs(NNAsyncWaitAPI* api);
void NNBuildObjectApiStubs(NNObjectAPI* api);
void NNBuildAssetRegistryApiStubs(NNAssetRegistryAPI* api);
void NNBuildAssetManagerApiStubs(NNAssetManagerAPI* api);
void NNBuildAssetCookerApiStubs(NNAssetCookerAPI* api);
void NNBuildEntityApiStubs(NNEntityAPI* api);
void NNBuildWindowApiStubs(NNWindowAPI* api);
void NNBuildVfsApiStubs(NNVfsAPI* api);
void NNBuildEventApiStubs(NNEventAPI* api);
void NNBuildRenderAssetApiStubs(NNRenderAssetAPI* api);
void NNBuildViewportRenderApiStubs(NNViewportRenderAPI* api);

#ifdef __cplusplus
} /* extern "C" */
#endif
