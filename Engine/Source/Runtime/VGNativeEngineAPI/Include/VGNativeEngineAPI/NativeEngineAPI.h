#pragma once

/**
 * @file NativeEngineAPI.h
 * @brief VGNativeEngineAPI 模組總包含檔（Engine Runtime ABI）。
 */

#include "VGNativeEngineAPI/AsyncWaitAPI.h"
#include "VGNativeEngineAPI/AudioAPI.h"
#include "VGNativeEngineAPI/AssetAPI.h"
#include "VGNativeEngineAPI/EngineAPIRegistry.h"
#include "VGNativeEngineAPI/EngineHandles.h"
#include "VGNativeEngineAPI/InputAPI.h"
#include "VGNativeEngineAPI/NativeInterop.h"
#include "VGNativeEngineAPI/RenderAPI.h"
#include "VGNativeEngineAPI/SceneAPI.h"
#include "VGNativeEngineAPI/TimingAPI.h"
#include "VGNativeEngineAPI/UIAPI.h"

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
