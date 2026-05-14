#pragma once

/**
 * @file AssetAPI.h
 * @brief 資源載入 **Engine Service** 函數表。
 */

#include "VGNativeEngineAPI/EngineHandles.h"
#include "VGNativeEngineAPI/NativeInterop.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef VGAssetHandle(VG_ENGINE_ABI_STDCALL* VGAssetLoadFn)(const char* virtualPathUtf8);

typedef void(VG_ENGINE_ABI_STDCALL* VGAssetUnloadFn)(VGAssetHandle asset);

typedef struct VGAssetAPI
{
	VGAssetLoadFn loadAsset;
	VGAssetUnloadFn unloadAsset;
} VGAssetAPI;

#ifdef __cplusplus
} /* extern "C" */
#endif
