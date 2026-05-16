#pragma once

/**
 * @file SceneAPI.h
 * @brief 場景 **Engine Service** 函數表（載入、實體生成與查詢）；**不**含 Gameplay 劇本語義。
 *
 * 字串參數：須為 **NUL 結尾 UTF-8**；nullptr 視為 no-op / 失敗回傳 0。
 * Phase 5：表尾追加層級、變換與命名（layout v3 子表擴充）。
 *
 * **與託管 ECS 及 EntityAPI 的邊界（簡體中文摘要）**
 * - 本表所涉 **VGEntityHandle** 僅表示 **場景圖／Prefab 實例** 控制碼，由 `spawn`/`destroy` 等場景 API 管理。
 * - **VisionGal.Managed.Entity** 之 **EntityHandle** 為純 C# ECS 首包，與 `VGEntityHandle` **無自動映射**。
 * - 未來 Native ECS 能力掛載於 **`VGNativeEngineAPI::entity`**（`EntityAPI.h`），與本 `VGSceneAPI` 子表分離設計。
 */

#include "NNNativeEngineAPI/EngineHandles.h"
#include "NNNativeEngineAPI/EngineTypes.h"
#include "NNNativeEngineAPI/NativeInterop.h"

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

typedef int(VG_ENGINE_ABI_STDCALL* VGSceneUnloadSceneFn)(const char* sceneNameUtf8);
typedef int(VG_ENGINE_ABI_STDCALL* VGSceneGetActiveSceneNameFn)(char* outUtf8, std::size_t outCapacity);
typedef void(VG_ENGINE_ABI_STDCALL* VGSceneSetParentFn)(VGEntityHandle child, VGEntityHandle parent);
typedef VGEntityHandle(VG_ENGINE_ABI_STDCALL* VGSceneGetParentFn)(VGEntityHandle entity);
typedef std::uint32_t(VG_ENGINE_ABI_STDCALL* VGSceneGetChildCountFn)(VGEntityHandle entity);
typedef VGEntityHandle(VG_ENGINE_ABI_STDCALL* VGSceneGetChildAtFn)(VGEntityHandle entity, std::uint32_t index);
typedef void(VG_ENGINE_ABI_STDCALL* VGSceneGetTransformFn)(VGEntityHandle entity, VGTransform3* outTransform);
typedef void(VG_ENGINE_ABI_STDCALL* VGSceneSetTransformFn)(VGEntityHandle entity, const VGTransform3* transform);
typedef int(VG_ENGINE_ABI_STDCALL* VGSceneSetEntityNameFn)(VGEntityHandle entity, const char* nameUtf8);
typedef int(VG_ENGINE_ABI_STDCALL* VGSceneGetEntityNameFn)(VGEntityHandle entity, char* outUtf8, std::size_t outCapacity);

typedef struct VGSceneAPI
{
	VGSceneLoadSceneFn loadScene;
	VGSceneSpawnFn spawn;
	VGSceneDestroyFn destroy;
	VGSceneFindFn find;
	VGSceneActivateFn activate;
	VGSceneUnloadSceneFn unloadScene;
	VGSceneGetActiveSceneNameFn getActiveSceneName;
	VGSceneSetParentFn setParent;
	VGSceneGetParentFn getParent;
	VGSceneGetChildCountFn getChildCount;
	VGSceneGetChildAtFn getChildAt;
	VGSceneGetTransformFn getTransform;
	VGSceneSetTransformFn setTransform;
	VGSceneSetEntityNameFn setEntityName;
	VGSceneGetEntityNameFn getEntityName;
} VGSceneAPI;

#ifdef __cplusplus
} /* extern "C" */
#endif
