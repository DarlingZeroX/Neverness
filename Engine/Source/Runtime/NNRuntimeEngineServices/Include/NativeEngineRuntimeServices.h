#pragma once

#include <stdbool.h>
#include "NNNativeEngineAPI/Include/EngineAPIRegistry.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 以 **Stub 基底表** 覆寫 Timing / Async / Scene 擴充欄位 / Asset 擴充欄位，指向 `NNEngineRuntime`。
 */
void NNNativeEngineApiTable_BuildRuntime(NNNativeEngineAPI* outTable);

/** @brief 行程靜態單例：與 `NNNativeEngineApi_GetDefaultTable` 相同生命週期語意。 */
const NNNativeEngineAPI* NNNativeEngineApi_GetRuntimeTable(void);

bool NNEngineRuntimeHost_Initialize(void);
void NNEngineRuntimeHost_Tick(float deltaTimeSeconds);
void NNEngineRuntimeHost_Shutdown(void);

#ifdef __cplusplus
}
#endif
