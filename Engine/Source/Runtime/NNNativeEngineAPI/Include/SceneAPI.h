#pragma once

/**
 * @file SceneAPI.h
 * @brief 場景 **Engine Service** 函數表（載入、實體生成與查詢）；**不**含 Gameplay 劇本語義。
 *
 * 字串參數：須為 **NUL 結尾 UTF-8**；nullptr 視為 no-op / 失敗回傳 0。
 * Phase 5：表尾追加層級、變換與命名（layout v3 子表擴充）。
 *
 * **與託管 ECS 及 EntityAPI 的邊界（簡體中文摘要）**
 * - 本表所涉 **NNEntityHandle** 僅表示 **場景圖／Prefab 實例** 控制碼，由 `spawn`/`destroy` 等場景 API 管理。
 * - 託管 **Neverness.Managed.Scene.SceneEntity** 為 **NNEntityHandle** 薄門面，與本表一一對應，**不**在 C# 側複製場景存儲。
 * - 未來 Native ECS 能力掛載於 **`NNNativeEngineAPI::entity`**（`EntityAPI.h`），與本 `NNSceneAPI` 子表分離設計。
 */

#include "EngineHandles.h"
#include "EngineTypes.h"
#include "NativeInterop.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int(NN_ENGINE_ABI_STDCALL* NNSceneLoadSceneFn)(const char* sceneNameUtf8);

/** @brief 由虛擬路徑生成實體；失敗回傳 0。 */
typedef NNEntityHandle(NN_ENGINE_ABI_STDCALL* NNSceneSpawnFn)(const char* prefabVirtualPathUtf8);

typedef void(NN_ENGINE_ABI_STDCALL* NNSceneDestroyFn)(NNEntityHandle entity);

/** @brief 依名稱尋找實體；未找到回傳 0。 */
typedef NNEntityHandle(NN_ENGINE_ABI_STDCALL* NNSceneFindFn)(const char* entityNameUtf8);

/** @brief `active` 非 0 表啟用，0 表停用（銷毀請用 destroy）。 */
typedef void(NN_ENGINE_ABI_STDCALL* NNSceneActivateFn)(NNEntityHandle entity, int active);

typedef int(NN_ENGINE_ABI_STDCALL* NNSceneUnloadSceneFn)(const char* sceneNameUtf8);
typedef int(NN_ENGINE_ABI_STDCALL* NNSceneGetActiveSceneNameFn)(char* outUtf8, std::size_t outCapacity);
typedef void(NN_ENGINE_ABI_STDCALL* NNSceneSetParentFn)(NNEntityHandle child, NNEntityHandle parent);
typedef NNEntityHandle(NN_ENGINE_ABI_STDCALL* NNSceneGetParentFn)(NNEntityHandle entity);
typedef std::uint32_t(NN_ENGINE_ABI_STDCALL* NNSceneGetChildCountFn)(NNEntityHandle entity);
typedef NNEntityHandle(NN_ENGINE_ABI_STDCALL* NNSceneGetChildAtFn)(NNEntityHandle entity, std::uint32_t index);
typedef void(NN_ENGINE_ABI_STDCALL* NNSceneGetTransformFn)(NNEntityHandle entity, NNTransform3* outTransform);
typedef void(NN_ENGINE_ABI_STDCALL* NNSceneSetTransformFn)(NNEntityHandle entity, const NNTransform3* transform);
typedef int(NN_ENGINE_ABI_STDCALL* NNSceneSetEntityNameFn)(NNEntityHandle entity, const char* nameUtf8);
typedef int(NN_ENGINE_ABI_STDCALL* NNSceneGetEntityNameFn)(NNEntityHandle entity, char* outUtf8, std::size_t outCapacity);

typedef struct NNSceneAPI
{
	NNSceneLoadSceneFn loadScene;
	NNSceneSpawnFn spawn;
	NNSceneDestroyFn destroy;
	NNSceneFindFn find;
	NNSceneActivateFn activate;
	NNSceneUnloadSceneFn unloadScene;
	NNSceneGetActiveSceneNameFn getActiveSceneName;
	NNSceneSetParentFn setParent;
	NNSceneGetParentFn getParent;
	NNSceneGetChildCountFn getChildCount;
	NNSceneGetChildAtFn getChildAt;
	NNSceneGetTransformFn getTransform;
	NNSceneSetTransformFn setTransform;
	NNSceneSetEntityNameFn setEntityName;
	NNSceneGetEntityNameFn getEntityName;
} NNSceneAPI;

#ifdef __cplusplus
} /* extern "C" */
#endif
