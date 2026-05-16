#pragma once

/**
 * @file AssetRegistryAPI.h
 * @brief 專案資產登記表 ABI（GUID ↔ 虛擬路徑；與 `VGAssetAPI` 執行時載入分離）。
 *
 * 字串輸出函數：寫入 `outUtf8`（容量 `outCapacity`），回傳寫入字節數；失敗 -1。
 */

#include "NNNativeEngineAPI/EngineTypes.h"
#include "NNNativeEngineAPI/NativeInterop.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int(VG_ENGINE_ABI_STDCALL* VGAssetRegistryRegisterFn)(const char* virtualPathUtf8, VGGuid guid);
typedef int(VG_ENGINE_ABI_STDCALL* VGAssetRegistryUnregisterByGuidFn)(VGGuid guid);
typedef int(VG_ENGINE_ABI_STDCALL* VGAssetRegistryUnregisterByPathFn)(const char* virtualPathUtf8);
typedef int(VG_ENGINE_ABI_STDCALL* VGAssetRegistryResolvePathByGuidFn)(
	VGGuid guid,
	char* outUtf8,
	std::size_t outCapacity);
typedef int(VG_ENGINE_ABI_STDCALL* VGAssetRegistryResolveGuidByPathFn)(const char* virtualPathUtf8, VGGuid* outGuid);
typedef std::uint32_t(VG_ENGINE_ABI_STDCALL* VGAssetRegistryGetDependencyCountFn)(VGGuid guid);
typedef int(VG_ENGINE_ABI_STDCALL* VGAssetRegistryGetDependencyAtFn)(
	VGGuid guid,
	std::uint32_t index,
	VGGuid* outDependency);
typedef VGGuid(VG_ENGINE_ABI_STDCALL* VGAssetRegistryImportAssetFn)(const char* virtualPathUtf8);

typedef struct VGAssetRegistryAPI
{
	VGAssetRegistryRegisterFn registerAsset;
	VGAssetRegistryUnregisterByGuidFn unregisterByGuid;
	VGAssetRegistryUnregisterByPathFn unregisterByPath;
	VGAssetRegistryResolvePathByGuidFn resolvePathByGuid;
	VGAssetRegistryResolveGuidByPathFn resolveGuidByPath;
	VGAssetRegistryGetDependencyCountFn getDependencyCount;
	VGAssetRegistryGetDependencyAtFn getDependencyAt;
	VGAssetRegistryImportAssetFn importAsset;
} VGAssetRegistryAPI;

#ifdef __cplusplus
} /* extern "C" */
#endif
