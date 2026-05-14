#pragma once

/**
 * @file SceneAPI.h
 * @brief 場景載入 **Engine Service** 函數表。
 *
 * @return 非零表請求已接受（非同步完成）；0 表失敗。Stub 固定回傳 1。
 */

#include "VGNativeEngineAPI/NativeInterop.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int(VG_ENGINE_ABI_STDCALL* VGSceneLoadSceneFn)(const char* sceneNameUtf8);

typedef struct VGSceneAPI
{
	VGSceneLoadSceneFn loadScene;
} VGSceneAPI;

#ifdef __cplusplus
} /* extern "C" */
#endif
