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

typedef struct NNAssetRegistryAPI
{
	NNAssetRegistryRegisterFn registerAsset;
	NNAssetRegistryUnregisterByGuidFn unregisterByGuid;
	NNAssetRegistryUnregisterByPathFn unregisterByPath;
	NNAssetRegistryResolvePathByGuidFn resolvePathByGuid;
	NNAssetRegistryResolveGuidByPathFn resolveGuidByPath;
	NNAssetRegistryGetDependencyCountFn getDependencyCount;
	NNAssetRegistryGetDependencyAtFn getDependencyAt;
	NNAssetRegistryImportAssetFn importAsset;
} NNAssetRegistryAPI;

#ifdef __cplusplus
} /* extern "C" */
#endif
