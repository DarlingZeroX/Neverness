#pragma once

/**
 * @file TimingAPI.h
 * @brief 時間 **Engine Service** 函數表（秒為單位；帧序為單調遞增整數）。
 *
 * Phase 4：表尾追加總時間與帧索引查詢；由 **VGEngineRuntime::Tick** 驅動更新語意。
 */

#include "NativeInterop.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef float(NN_ENGINE_ABI_STDCALL* NNTimingGetDeltaTimeFn)(void);

/** @brief 自 Runtime **Initialize** 以來累積之模擬/實時間（秒），語意由實作定義。 */
typedef float(NN_ENGINE_ABI_STDCALL* NNTimingGetTotalTimeFn)(void);

/** @brief 已執行之 **Tick** 次數（從 0 起算，每 Tick 遞增）。 */
typedef std::uint64_t(NN_ENGINE_ABI_STDCALL* NNTimingGetFrameIndexFn)(void);

typedef struct NNTimingAPI
{
	NNTimingGetDeltaTimeFn getDeltaTime;
	NNTimingGetTotalTimeFn getTotalTime;
	NNTimingGetFrameIndexFn getFrameIndex;
} NNTimingAPI;

#ifdef __cplusplus
} /* extern "C" */
#endif
