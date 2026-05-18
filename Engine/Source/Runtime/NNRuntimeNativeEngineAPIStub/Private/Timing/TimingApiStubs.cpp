/**
 * @file TimingApiStubs.cpp
 * @brief **NNTimingAPI** 預設 Stub：時間與幀索引恆為零。
 */

#include "Common/StubInvokeCounter.h"
#include "Internal/ApiStubBuilders.h"

#include "NNNativeEngineAPI/Include/NativeInterop.h"

namespace
{
float NN_ENGINE_ABI_STDCALL stub_timing_getDeltaTime(void)
{
	NN::StubRuntime::BumpInvokeCount();
	return 0.f;
}

float NN_ENGINE_ABI_STDCALL stub_timing_getTotalTime(void)
{
	NN::StubRuntime::BumpInvokeCount();
	return 0.f;
}

std::uint64_t NN_ENGINE_ABI_STDCALL stub_timing_getFrameIndex(void)
{
	NN::StubRuntime::BumpInvokeCount();
	return 0;
}
} // namespace

extern "C" void NNBuildTimingApiStubs(NNTimingAPI* api)
{
	if (api == nullptr)
	{
		return;
	}
	api->getDeltaTime = &stub_timing_getDeltaTime;
	api->getTotalTime = &stub_timing_getTotalTime;
	api->getFrameIndex = &stub_timing_getFrameIndex;
}
