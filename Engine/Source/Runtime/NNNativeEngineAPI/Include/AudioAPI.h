#pragma once

/**
 * @file AudioAPI.h
 * @brief 音訊 **Engine Service** 函數表（路徑皆為 NUL 結尾 UTF-8）。
 */

#include "EngineHandles.h"
#include "NativeInterop.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief 播放 BGM；回傳非零 voice/bgm 控制代碼，失敗為 0（Stub 固定非零合成值）。 */
typedef NNAudioHandle(NN_ENGINE_ABI_STDCALL* NNAudioPlayBgmFn)(const char* assetPathUtf8);

typedef NNAudioHandle(NN_ENGINE_ABI_STDCALL* NNAudioPlayVoiceFn)(const char* assetPathUtf8);

typedef struct NNAudioAPI
{
	NNAudioPlayBgmFn playBgm;
	NNAudioPlayVoiceFn playVoice;
} NNAudioAPI;

#ifdef __cplusplus
} /* extern "C" */
#endif
