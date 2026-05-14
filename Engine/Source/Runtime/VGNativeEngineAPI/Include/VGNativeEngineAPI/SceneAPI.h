#pragma once

/**
 * @file SceneAPI.h
 * @brief 場景 **Engine Service** 函數表（載入、實體生成與查詢）；**不**含 Gameplay 劇本語義。
 *
 * 字串參數：須為 **NUL 結尾 UTF-8**；nullptr 視為 no-op / 失敗回傳 0。
 * Phase 4：表尾追加 spawn / destroy / find / activate；未接線實作可回傳 0 Handle。
 */

#include "VGNativeEngineAPI/EngineHandles.h"
#include "VGNativeEngineAPI/NativeInterop.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int(VG_ENGINE_ABI_STDCALL* VGSceneLoadSceneFn)(const char* sceneNameUtf8);

/** @brief 由虛擬路徑生成實體；失敗回傳 0。 */
typedef VGEntityHandle(VG_ENGINE_ABI_STDCALL* VGSceneSpawnFn)(const char* prefabVirtualPathUtf8);

typedef void(VG_ENGINE_ABI_STDCALL* VGSceneDestroyFn)(VGEntityHandle entity);

/** @brief 依名稱尋找實體；未找到回傳 0。 */
typedef VGEntityHandle(VG_ENGINE_ABI_STDCALL* VGSceneFindFn)(const char* entityNameUtf8);

/** @brief `active` 非 0 表啟用，0 表停用（銷毀請用 destroy）。 */
typedef void(VG_ENGINE_ABI_STDCALL* VGSceneActivateFn)(VGEntityHandle entity, int active);

typedef struct VGSceneAPI
{
	VGSceneLoadSceneFn loadScene;
	VGSceneSpawnFn spawn;
	VGSceneDestroyFn destroy;
	VGSceneFindFn find;
	VGSceneActivateFn activate;
} VGSceneAPI;

#ifdef __cplusplus
} /* extern "C" */
#endif
