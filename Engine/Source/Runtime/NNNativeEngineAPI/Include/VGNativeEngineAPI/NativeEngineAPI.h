#pragma once

/**
 * @file NativeEngineAPI.h
 * @brief VGNativeEngineAPI 模組總包含檔（Engine Runtime ABI）。
 */

#include "NNNativeEngineAPI/AsyncWaitAPI.h"
#include "NNNativeEngineAPI/AudioAPI.h"
#include "NNNativeEngineAPI/AssetAPI.h"
#include "NNNativeEngineAPI/EngineAPIRegistry.h"
#include "NNNativeEngineAPI/EngineHandles.h"
#include "NNNativeEngineAPI/InputAPI.h"
#include "NNNativeEngineAPI/NativeInterop.h"
#include "NNNativeEngineAPI/RenderAPI.h"
#include "NNNativeEngineAPI/SceneAPI.h"
#include "NNNativeEngineAPI/TimingAPI.h"
#include "NNNativeEngineAPI/UIAPI.h"

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
