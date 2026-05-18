#pragma once

/**
 * @file AssetAPI.h
 * @brief 資源載入 **Engine Service** 函數表。
 *
 * Phase 4：在既有 `loadAsset` / `unloadAsset` 之後追加紋理與音訊快捷載入；未接線時回傳 0 Handle。
 */

#include "EngineHandles.h"
#include "NativeInterop.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef NNAssetHandle(NN_ENGINE_ABI_STDCALL* NNAssetLoadFn)(const char* virtualPathUtf8);

typedef void(NN_ENGINE_ABI_STDCALL* NNAssetUnloadFn)(NNAssetHandle asset);

typedef NNTextureHandle(NN_ENGINE_ABI_STDCALL* NNAssetLoadTextureFn)(const char* virtualPathUtf8);

typedef NNAudioHandle(NN_ENGINE_ABI_STDCALL* NNAssetLoadAudioFn)(const char* virtualPathUtf8);

typedef struct NNAssetAPI
{
	NNAssetLoadFn loadAsset;
	NNAssetUnloadFn unloadAsset;
	NNAssetLoadTextureFn loadTexture;
	NNAssetLoadAudioFn loadAudio;
} NNAssetAPI;

#ifdef __cplusplus
} /* extern "C" */
#endif
