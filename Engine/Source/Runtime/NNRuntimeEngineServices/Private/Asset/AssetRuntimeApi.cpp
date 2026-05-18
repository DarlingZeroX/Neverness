/**
 * @file AssetRuntimeApi.cpp
 * @brief **NNAssetAPI** 部分欄位 Runtime 轉發（紋理／音訊快捷項）。
 */

#include "Internal/RuntimeApiBuilders.h"

#include "NNNativeEngineAPI/Include/NativeInterop.h"
#include "NNRuntimeEngine/Include/NNEngineRuntime.h"

namespace
{
using NN::Runtime::engine::NNEngineRuntime;

NNTextureHandle NN_ENGINE_ABI_STDCALL rt_asset_loadTexture(const char* virtualPathUtf8)
{
	return NNEngineRuntime::Instance().Asset().LoadTexture(virtualPathUtf8);
}

NNAudioHandle NN_ENGINE_ABI_STDCALL rt_asset_loadAudio(const char* virtualPathUtf8)
{
	return NNEngineRuntime::Instance().Asset().LoadAudio(virtualPathUtf8);
}
} // namespace

extern "C" void NNBuildAssetRuntimeApi(NNAssetAPI* api)
{
	if (api == nullptr)
	{
		return;
	}
	api->loadTexture = &rt_asset_loadTexture;
	api->loadAudio = &rt_asset_loadAudio;
}
