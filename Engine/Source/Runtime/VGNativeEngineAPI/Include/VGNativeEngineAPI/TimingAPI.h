#pragma once

/**
 * @file TimingAPI.h
 * @brief 時間 **Engine Service** 函數表（秒為單位之帧間隔）。
 */

#include "VGNativeEngineAPI/NativeInterop.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef float(VG_ENGINE_ABI_STDCALL* VGTimingGetDeltaTimeFn)(void);

typedef struct VGTimingAPI
{
	VGTimingGetDeltaTimeFn getDeltaTime;
} VGTimingAPI;

#ifdef __cplusplus
} /* extern "C" */
#endif
