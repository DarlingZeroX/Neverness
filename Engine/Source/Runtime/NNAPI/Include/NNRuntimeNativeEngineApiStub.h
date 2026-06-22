#pragma once

/**
 * @file NNRuntimeNativeEngineApiStub.h
 * @brief Stub Runtime 模組對外 C 入口：預設引擎服務表、行程單例與測試計數。
 *
 * 本模組為 Editor / ManagedHost / 單元測試提供「虛擬 Runtime」，不連結 **NNEngineRuntime**。
 * 純 ABI 契約（POD、子表 typedef）見 **NNNativeEngineAPI**。
 */

#include "Engine/EngineAPIRegistry.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 將 `outTable` 清零後填入預設 Stub 函數指標（不連結 VGEngine）。
 */
void NNNativeEngineApiTable_BuildDefault(NNNativeEngineAPI* outTable);

/**
 * @brief 取得行程內單例預設引擎服務表（指標生命週期：進程靜態，永不失效）。
 */
const NNNativeEngineAPI* NNNativeEngineApi_GetDefaultTable(void);

/**
 * @brief 供測試斷言：累計 Stub 實作被呼叫次數（跨子系統加總）。
 */
std::uint32_t NNNativeEngineApi_GetStubInvokeCount(void);

#ifdef __cplusplus
} /* extern "C" */
#endif
