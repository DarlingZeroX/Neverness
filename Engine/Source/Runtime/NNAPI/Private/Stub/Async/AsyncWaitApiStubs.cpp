/**
 * @file AsyncWaitApiStubs.cpp
 * @brief **NNAsyncWaitAPI** Stub：建立即視為可一次 **tryComplete** 的「已完成」語意。
 */

#include "Common/StubInvokeCounter.h"
#include "Internal/ApiStubBuilders.h"

#include "Engine/NativeInterop.h"

namespace
{
NNAsyncWaitHandle NN_ENGINE_ABI_STDCALL stub_async_createWait(void)
{
	NN::StubRuntime::BumpInvokeCount();
	// 非零即代表有效等待控制代碼；Stub 不維護狀態機。
	return 1;
}

int NN_ENGINE_ABI_STDCALL stub_async_tryComplete(NNAsyncWaitHandle wait)
{
	NN::StubRuntime::BumpInvokeCount();
	return (wait != 0) ? 1 : 0;
}

void NN_ENGINE_ABI_STDCALL stub_async_releaseWait(NNAsyncWaitHandle wait)
{
	NN::StubRuntime::BumpInvokeCount();
	(void)wait;
}
} // namespace

extern "C" void NNBuildAsyncWaitApiStubs(NNAsyncWaitAPI* api)
{
	if (api == nullptr)
	{
		return;
	}
	api->createWait = &stub_async_createWait;
	api->tryComplete = &stub_async_tryComplete;
	api->releaseWait = &stub_async_releaseWait;
}
