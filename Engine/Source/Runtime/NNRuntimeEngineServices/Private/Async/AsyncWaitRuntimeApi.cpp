/**
 * @file AsyncWaitRuntimeApi.cpp
 * @brief **NNAsyncWaitAPI** Runtime 轉發至 **NNEngineRuntime::Async()**。
 */

#include "Internal/RuntimeApiBuilders.h"

#include "Engine/NativeInterop.h"
#include "NNRuntimeEngine/Include/NNEngineRuntime.h"

namespace
{
using NN::Runtime::engine::NNEngineRuntime;

NNAsyncWaitHandle NN_ENGINE_ABI_STDCALL rt_async_createWait(void)
{
	return static_cast<NNAsyncWaitHandle>(NNEngineRuntime::Instance().Async().CreateWait());
}

int NN_ENGINE_ABI_STDCALL rt_async_tryComplete(NNAsyncWaitHandle wait)
{
	return NNEngineRuntime::Instance().Async().TryComplete(static_cast<std::uint64_t>(wait));
}

void NN_ENGINE_ABI_STDCALL rt_async_releaseWait(NNAsyncWaitHandle wait)
{
	NNEngineRuntime::Instance().Async().ReleaseWait(static_cast<std::uint64_t>(wait));
}
} // namespace

extern "C" void NNBuildAsyncWaitRuntimeApi(NNAsyncWaitAPI* api)
{
	if (api == nullptr)
	{
		return;
	}
	api->createWait = &rt_async_createWait;
	api->tryComplete = &rt_async_tryComplete;
	api->releaseWait = &rt_async_releaseWait;
}
