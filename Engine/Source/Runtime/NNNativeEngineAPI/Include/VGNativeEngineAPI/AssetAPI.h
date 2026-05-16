#pragma once

/**
 * @file AssetAPI.h
 * @brief 資源載入 **Engine Service** 函數表。
 *
 * Phase 4：在既有 `loadAsset` / `unloadAsset` 之後追加紋理與音訊快捷載入；未接線時回傳 0 Handle。
 */

#include "NNNativeEngineAPI/EngineHandles.h"
#include "NNNativeEngineAPI/NativeInterop.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef VGAssetHandle(VG_ENGINE_ABI_STDCALL* VGAssetLoadFn)(const char* virtualPathUtf8);

typedef void(VG_ENGINE_ABI_STDCALL* VGAssetUnloadFn)(VGAssetHandle asset);

typedef VGTextureHandle(VG_ENGINE_ABI_STDCALL* VGAssetLoadTextureFn)(const char* virtualPathUtf8);

typedef VGAudioHandle(VG_ENGINE_ABI_STDCALL* VGAssetLoadAudioFn)(const char* virtualPathUtf8);

typedef struct VGAssetAPI
{
	VGAssetLoadFn loadAsset;
	VGAssetUnloadFn unloadAsset;
	VGAssetLoadTextureFn loadTexture;
	VGAssetLoadAudioFn loadAudio;
} VGAssetAPI;

#ifdef __cplusplus
} /* extern "C" */
#endif
