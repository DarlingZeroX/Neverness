/**
 * @file TimingRuntimeApi.cpp
 * @brief **NNTimingAPI** Runtime 轉發至 **NNEngineRuntime::Timing()**。
 */

#include "Internal/RuntimeApiBuilders.h"

#include "NNNativeEngineAPI/Include/NativeInterop.h"
#include "NNRuntimeEngine/Include/NNEngineRuntime.h"

namespace
{
using NN::Runtime::engine::NNEngineRuntime;

float NN_ENGINE_ABI_STDCALL rt_timing_getDeltaTime(void)
{
	return NNEngineRuntime::Instance().Timing().GetDeltaTime();
}

float NN_ENGINE_ABI_STDCALL rt_timing_getTotalTime(void)
{
	return NNEngineRuntime::Instance().Timing().GetTotalTime();
}

std::uint64_t NN_ENGINE_ABI_STDCALL rt_timing_getFrameIndex(void)
{
	return NNEngineRuntime::Instance().Timing().GetFrameIndex();
}
} // namespace

extern "C" void NNBuildTimingRuntimeApi(NNTimingAPI* api)
{
	if (api == nullptr)
	{
		return;
	}
	api->getDeltaTime = &rt_timing_getDeltaTime;
	api->getTotalTime = &rt_timing_getTotalTime;
	api->getFrameIndex = &rt_timing_getFrameIndex;
}
