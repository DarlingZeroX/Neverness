/**
 * @file AudioApiStubs.cpp
 * @brief **NNAudioAPI** 預設 Stub：不播放音訊，返回無效 **NNAudioHandle**。
 */

#include "Common/StubInvokeCounter.h"
#include "Internal/ApiStubBuilders.h"

#include "Engine/NativeInterop.h"

namespace
{
NNAudioHandle NN_ENGINE_ABI_STDCALL stub_audio_playBgm(const char* assetPathUtf8)
{
	NN::StubRuntime::BumpInvokeCount();
	(void)assetPathUtf8;
	return 0;
}

NNAudioHandle NN_ENGINE_ABI_STDCALL stub_audio_playVoice(const char* assetPathUtf8)
{
	NN::StubRuntime::BumpInvokeCount();
	(void)assetPathUtf8;
	return 0;
}
} // namespace

extern "C" void NNBuildAudioApiStubs(NNAudioAPI* api)
{
	if (api == nullptr)
	{
		return;
	}
	api->playBgm = &stub_audio_playBgm;
	api->playVoice = &stub_audio_playVoice;
}
