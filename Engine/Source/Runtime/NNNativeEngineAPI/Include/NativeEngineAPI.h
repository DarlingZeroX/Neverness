#pragma once

/**
 * @file NativeEngineAPI.h
 * @brief VGNativeEngineAPI 模組總包含檔（Engine Runtime ABI）。
 */

#include "AsyncWaitAPI.h"
#include "AudioAPI.h"
#include "AssetAPI.h"
#include "EngineAPIRegistry.h"
#include "EngineHandles.h"
#include "InputAPI.h"
#include "NativeInterop.h"
#include "RenderAPI.h"
#include "SceneAPI.h"
#include "TimingAPI.h"
#include "UIAPI.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 取得行程內單例預設引擎服務表（指標生命週期：進程靜態，永不失效）。
 */
VG_NATIVE_ENGINE_API const VGNativeEngineAPI* VGNativeEngineApi_GetDefaultTable(void);

#ifdef __cplusplus
} /* extern "C" */
#endif
