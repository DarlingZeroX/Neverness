#pragma once

/**
 * @file InputAPI.h
 * @brief 輸入查詢 **Engine Service** 函數表。
 *
 * `keyCode` 之枚舉定義由引擎統一表驅動（本 Phase 不展開）；Stub 固定回傳 0（未按下）。
 */

#include "NativeInterop.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief 非零表已按下，0 表未按下。 */
typedef int(VG_ENGINE_ABI_STDCALL* VGInputIsKeyPressedFn)(int keyCode);

typedef struct VGInputAPI
{
	VGInputIsKeyPressedFn isKeyPressed;
} VGInputAPI;

#ifdef __cplusplus
} /* extern "C" */
#endif
