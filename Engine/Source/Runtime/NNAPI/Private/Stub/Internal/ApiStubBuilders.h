#pragma once

/**
 * @file ApiStubBuilders.h
 * @brief 各 Engine Service 子表之 Stub 建表函式宣告（僅寫入函數指標，不修改 **layoutVersion**）。
 */

#include "Engine/AsyncWaitAPI.h"
#include "Engine/AudioAPI.h"
#include "Engine/RenderAPI.h"
#include "Engine/VfsAPI.h"
#include "Engine/EventAPI.h"
#include "Engine/ViewportRenderAPI.h"

#ifdef __cplusplus
extern "C" {
#endif

void NNBuildRenderApiStubs(NNRenderAPI* api);
void NNBuildAudioApiStubs(NNAudioAPI* api);
void NNBuildAsyncWaitApiStubs(NNAsyncWaitAPI* api);
void NNBuildVfsApiStubs(NNVfsAPI* api);
void NNBuildEventApiStubs(NNEventAPI* api);
void NNBuildViewportRenderApiStubs(NNViewportRenderAPI* api);

#ifdef __cplusplus
} /* extern "C" */
#endif
