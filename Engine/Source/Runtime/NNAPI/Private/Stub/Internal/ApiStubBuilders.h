#pragma once

/**
 * @file ApiStubBuilders.h
 * @brief 各 Engine Service 子表之 Stub 建表函式宣告（僅寫入函數指標，不修改 **layoutVersion**）。
 */

#include "Engine/ApplicationAPI.h"
#include "Engine/AsyncWaitAPI.h"
#include "Engine/AudioAPI.h"
#include "Engine/InputAPI.h"
#include "Engine/RenderAPI.h"
#include "Engine/VfsAPI.h"
#include "Engine/WindowAPI.h"
#include "Engine/EventAPI.h"
#include "Engine/RenderAssetAPI.h"
#include "Engine/ViewportRenderAPI.h"

#ifdef __cplusplus
extern "C" {
#endif

void NNBuildApplicationApiStubs(NNApplicationAPI* api);
void NNBuildRenderApiStubs(NNRenderAPI* api);
void NNBuildAudioApiStubs(NNAudioAPI* api);
void NNBuildInputApiStubs(NNInputAPI* api);
void NNBuildAsyncWaitApiStubs(NNAsyncWaitAPI* api);
void NNBuildWindowApiStubs(NNWindowAPI* api);
void NNBuildVfsApiStubs(NNVfsAPI* api);
void NNBuildEventApiStubs(NNEventAPI* api);
void NNBuildRenderAssetApiStubs(NNRenderAssetAPI* api);
void NNBuildViewportRenderApiStubs(NNViewportRenderAPI* api);

#ifdef __cplusplus
} /* extern "C" */
#endif
