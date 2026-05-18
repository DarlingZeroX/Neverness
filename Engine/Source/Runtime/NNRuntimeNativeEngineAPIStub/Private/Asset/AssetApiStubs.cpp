/**
 * @file AssetApiStubs.cpp
 * @brief **NNAssetAPI** 預設 Stub：載入／卸載為 no-op，不觸及真實資源管線。
 */

#include "Common/StubInvokeCounter.h"
#include "Internal/ApiStubBuilders.h"

#include "NNNativeEngineAPI/Include/NativeInterop.h"

namespace
{
NNAssetHandle NN_ENGINE_ABI_STDCALL stub_asset_load(const char* virtualPathUtf8)
{
	NN::StubRuntime::BumpInvokeCount();
	(void)virtualPathUtf8;
	return 0;
}

void NN_ENGINE_ABI_STDCALL stub_asset_unload(NNAssetHandle asset)
{
	NN::StubRuntime::BumpInvokeCount();
	(void)asset;
}

NNTextureHandle NN_ENGINE_ABI_STDCALL stub_asset_loadTexture(const char* virtualPathUtf8)
{
	NN::StubRuntime::BumpInvokeCount();
	(void)virtualPathUtf8;
	return 0;
}

NNAudioHandle NN_ENGINE_ABI_STDCALL stub_asset_loadAudio(const char* virtualPathUtf8)
{
	NN::StubRuntime::BumpInvokeCount();
	(void)virtualPathUtf8;
	return 0;
}
} // namespace

extern "C" void NNBuildAssetApiStubs(NNAssetAPI* api)
{
	if (api == nullptr)
	{
		return;
	}
	api->loadAsset = &stub_asset_load;
	api->unloadAsset = &stub_asset_unload;
	api->loadTexture = &stub_asset_loadTexture;
	api->loadAudio = &stub_asset_loadAudio;
}
