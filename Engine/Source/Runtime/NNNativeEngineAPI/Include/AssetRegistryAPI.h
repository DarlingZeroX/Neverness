#pragma once

/**
 * @file AssetRegistryAPI.h
 * @brief 專案資產登記表 ABI（GUID ↔ 虛擬路徑；與 `NNAssetAPI` 執行時載入分離）。
 *
 * 字串輸出函數：寫入 `outUtf8`（容量 `outCapacity`），回傳寫入字節數；失敗 -1。
 */

#include <cstdio>
#include "EngineTypes.h"
#include "NativeInterop.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int(NN_ENGINE_ABI_STDCALL* NNAssetRegistryRegisterFn)(const char* virtualPathUtf8, NNGuid guid);
typedef int(NN_ENGINE_ABI_STDCALL* NNAssetRegistryUnregisterByGuidFn)(NNGuid guid);
typedef int(NN_ENGINE_ABI_STDCALL* NNAssetRegistryUnregisterByPathFn)(const char* virtualPathUtf8);
typedef int(NN_ENGINE_ABI_STDCALL* NNAssetRegistryResolvePathByGuidFn)(
	NNGuid guid,
	char* outUtf8,
	std::size_t outCapacity);
typedef int(NN_ENGINE_ABI_STDCALL* NNAssetRegistryResolveGuidByPathFn)(const char* virtualPathUtf8, NNGuid* outGuid);
typedef std::uint32_t(NN_ENGINE_ABI_STDCALL* NNAssetRegistryGetDependencyCountFn)(NNGuid guid);
typedef int(NN_ENGINE_ABI_STDCALL* NNAssetRegistryGetDependencyAtFn)(
	NNGuid guid,
	std::uint32_t index,
	NNGuid* outDependency);
typedef NNGuid(NN_ENGINE_ABI_STDCALL* NNAssetRegistryImportAssetFn)(const char* virtualPathUtf8);

/** @brief 設定資產完整依賴列表（替換舊列表）。成功回傳 0。 */
typedef int(NN_ENGINE_ABI_STDCALL* NNAssetRegistrySetDependenciesFn)(NNGuid guid, const NNGuid* deps, std::uint32_t count);

/** @brief 新增單個依賴。成功回傳 0。 */
typedef int(NN_ENGINE_ABI_STDCALL* NNAssetRegistryAddDependencyFn)(NNGuid guid, NNGuid dependency);

/** @brief 移除單個依賴。成功回傳 0。 */
typedef int(NN_ENGINE_ABI_STDCALL* NNAssetRegistryRemoveDependencyFn)(NNGuid guid, NNGuid dependency);

/** @brief 取得反向依賴數量（哪些資產引用了 guid）。 */
typedef std::uint32_t(NN_ENGINE_ABI_STDCALL* NNAssetRegistryGetReverseDependencyCountFn)(NNGuid guid);

/** @brief 取得反向依賴。成功回傳 0。 */
typedef int(NN_ENGINE_ABI_STDCALL* NNAssetRegistryGetReverseDependencyAtFn)(NNGuid guid, std::uint32_t index, NNGuid* outDep);

/** @brief 檢測依賴圖是否存在環。有環回傳 1，無環回傳 0。 */
typedef int(NN_ENGINE_ABI_STDCALL* NNAssetRegistryHasCycleFn)(void);

/** @brief 取得依賴圖中資產總數。 */
typedef std::uint32_t(NN_ENGINE_ABI_STDCALL* NNAssetRegistryGetAssetCountFn)(void);

/** @brief 取得依賴圖中邊（依賴關係）總數。 */
typedef std::uint32_t(NN_ENGINE_ABI_STDCALL* NNAssetRegistryGetEdgeCountFn)(void);

typedef struct NNAssetRegistryAPI
{
	/* --- 原有欄位（Phase 5，保持順序不變） --- */
	NNAssetRegistryRegisterFn registerAsset;
	NNAssetRegistryUnregisterByGuidFn unregisterByGuid;
	NNAssetRegistryUnregisterByPathFn unregisterByPath;
	NNAssetRegistryResolvePathByGuidFn resolvePathByGuid;
	NNAssetRegistryResolveGuidByPathFn resolveGuidByPath;
	NNAssetRegistryGetDependencyCountFn getDependencyCount;
	NNAssetRegistryGetDependencyAtFn getDependencyAt;
	NNAssetRegistryImportAssetFn importAsset;

	/* --- Phase 1 新增欄位（僅追加，禁止重排） --- */
	NNAssetRegistrySetDependenciesFn setDependencies;
	NNAssetRegistryAddDependencyFn addDependency;
	NNAssetRegistryRemoveDependencyFn removeDependency;
	NNAssetRegistryGetReverseDependencyCountFn getReverseDependencyCount;
	NNAssetRegistryGetReverseDependencyAtFn getReverseDependencyAt;
	NNAssetRegistryHasCycleFn hasCycle;
	NNAssetRegistryGetAssetCountFn getAssetCount;
	NNAssetRegistryGetEdgeCountFn getEdgeCount;
} NNAssetRegistryAPI;

#ifdef __cplusplus
} /* extern "C" */
#endif
