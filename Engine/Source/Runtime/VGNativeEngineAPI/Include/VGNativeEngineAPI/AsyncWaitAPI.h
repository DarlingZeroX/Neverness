#pragma once

/**
 * @file AsyncWaitAPI.h
 * @brief 非同步等待之 **Engine Service** 最小 ABI（輪詢完成 + 釋放）。
 *
 * Stub 語意：
 * - `createWait` 建立一個 **已處於完成狀態** 的等待物件，便於託管端驗證路徑而無需真實執行緒睡眠。
 * - `tryComplete` 對有效 Handle 第一次呼叫回傳 1（完成），之後回傳 0（無進展/已讀取）。
 * - `release` 釋放內部狀態；對 0 或未知 Handle 必須為 no-op。
 *
 * 未來可替換為 job graph / IO 完成埠，而 **不變更** 欄位順序與函數簽名（僅擴充 layoutVersion）。
 */

#include "VGNativeEngineAPI/EngineHandles.h"
#include "VGNativeEngineAPI/NativeInterop.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef VGAsyncWaitHandle(VG_ENGINE_ABI_STDCALL* VGAsyncCreateWaitFn)(void);

/** @brief 輪詢：1 表已完成；0 表尚未/已消費完成狀態。 */
typedef int(VG_ENGINE_ABI_STDCALL* VGAsyncTryCompleteFn)(VGAsyncWaitHandle wait);

typedef void(VG_ENGINE_ABI_STDCALL* VGAsyncReleaseWaitFn)(VGAsyncWaitHandle wait);

typedef struct VGAsyncWaitAPI
{
	VGAsyncCreateWaitFn createWait;
	VGAsyncTryCompleteFn tryComplete;
	VGAsyncReleaseWaitFn releaseWait;
} VGAsyncWaitAPI;

#ifdef __cplusplus
} /* extern "C" */
#endif
