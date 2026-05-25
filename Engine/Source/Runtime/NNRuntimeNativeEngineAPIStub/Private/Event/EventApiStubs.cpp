/**
 * @file EventApiStubs.cpp
 * @brief **NNEventAPI** Stub：无 SDL；用于 Editor 测试与默认表。
 */

#include "Common/StubInvokeCounter.h"
#include "Internal/ApiStubBuilders.h"

#include "NNNativeEngineAPI/Include/NativeInterop.h"
#include "NNNativeEngineAPI/Include/EventAPI.h"

namespace
{

std::uint32_t NN_ENGINE_ABI_STDCALL stub_event_pollEvent(NNEvent* /*outEvent*/)
{
	NN::StubRuntime::BumpInvokeCount();
	return 0;
}

std::uint32_t NN_ENGINE_ABI_STDCALL stub_event_peekEvent(NNEvent* /*outEvent*/)
{
	NN::StubRuntime::BumpInvokeCount();
	return 0;
}

std::uint32_t NN_ENGINE_ABI_STDCALL stub_event_waitEvent(NNEvent* /*outEvent*/, std::uint32_t /*timeoutMs*/)
{
	NN::StubRuntime::BumpInvokeCount();
	return 0;
}

std::uint32_t NN_ENGINE_ABI_STDCALL stub_event_getEventString(
	const NNEvent* /*event*/,
	const char** /*outPtr*/,
	std::uint16_t* /*outLen*/)
{
	NN::StubRuntime::BumpInvokeCount();
	return 0;
}

std::uint32_t NN_ENGINE_ABI_STDCALL stub_event_getQueueCount(void)
{
	NN::StubRuntime::BumpInvokeCount();
	return 0;
}

void NN_ENGINE_ABI_STDCALL stub_event_flushEvents(void)
{
	NN::StubRuntime::BumpInvokeCount();
}

std::uint32_t NN_ENGINE_ABI_STDCALL stub_event_pushUserEvent(const NNEvent* /*event*/)
{
	NN::StubRuntime::BumpInvokeCount();
	return 0;
}

} // namespace

extern "C" void NNBuildEventApiStubs(NNEventAPI* api)
{
	if (api == nullptr)
	{
		return;
	}

	api->size = static_cast<std::uint32_t>(sizeof(NNEventAPI));
	api->pollEvent = &stub_event_pollEvent;
	api->peekEvent = &stub_event_peekEvent;
	api->waitEvent = &stub_event_waitEvent;
	api->getEventString = &stub_event_getEventString;
	api->getQueueCount = &stub_event_getQueueCount;
	api->flushEvents = &stub_event_flushEvents;
	api->pushUserEvent = &stub_event_pushUserEvent;
}
