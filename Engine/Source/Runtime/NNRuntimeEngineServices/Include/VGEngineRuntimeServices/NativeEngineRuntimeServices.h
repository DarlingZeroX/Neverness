#pragma once

#include <stdbool.h>

#include "NNNativeEngineAPI/NativeEngineAPI.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 以 **Stub 基底表** 覆寫 Timing / Async / Scene 擴充欄位 / Asset 擴充欄位，指向 `VGEngineRuntime`。
 */
void VGNativeEngineApiTable_BuildRuntime(VGNativeEngineAPI* outTable);

/** @brief 行程靜態單例：與 `VGNativeEngineApi_GetDefaultTable` 相同生命週期語意。 */
const VGNativeEngineAPI* VGNativeEngineApi_GetRuntimeTable(void);

bool VGEngineRuntimeHost_Initialize(void);
void VGEngineRuntimeHost_Tick(float deltaTimeSeconds);
void VGEngineRuntimeHost_Shutdown(void);

#ifdef __cplusplus
}
#endif
