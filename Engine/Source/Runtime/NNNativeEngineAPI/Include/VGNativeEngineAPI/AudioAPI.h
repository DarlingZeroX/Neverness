#pragma once

/**
 * @file AudioAPI.h
 * @brief 音訊 **Engine Service** 函數表（路徑皆為 NUL 結尾 UTF-8）。
 */

#include "NNNativeEngineAPI/EngineHandles.h"
#include "NNNativeEngineAPI/NativeInterop.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief 播放 BGM；回傳非零 voice/bgm 控制代碼，失敗為 0（Stub 固定非零合成值）。 */
typedef VGAudioHandle(VG_ENGINE_ABI_STDCALL* VGAudioPlayBgmFn)(const char* assetPathUtf8);

typedef VGAudioHandle(VG_ENGINE_ABI_STDCALL* VGAudioPlayVoiceFn)(const char* assetPathUtf8);

typedef struct VGAudioAPI
{
	VGAudioPlayBgmFn playBgm;
	VGAudioPlayVoiceFn playVoice;
} VGAudioAPI;

#ifdef __cplusplus
} /* extern "C" */
#endif
