/**
 * @file BuildEventApi.cpp
 * @brief 导出 **NNEventAPI** 函数表（进程内 **EventQueue** + **RuntimeApplication** 单例）。
 */

#include "EventApiExport.h"

#include "Core/EventQueue.h"
#include "RuntimeApplicationInstance.h"

#include "NNNativeEngineAPI/Include/NativeInterop.h"
#include "NNNativeEngineAPI/Include/EventTypes.h"
#include "NNNativeEngineAPI/Include/EventAPI.h"

namespace
{

NN::Runtime::EventQueue& GetQueue()
{
	return NN::Runtime::Application::GetRuntimeApplicationInstance().GetEventQueue();
}

std::uint32_t NN_ENGINE_ABI_STDCALL EvPollEvent(NNEvent* outEvent)
{
	if (outEvent == nullptr)
		return 0;
	return GetQueue().Pop(*outEvent) ? 1u : 0u;
}

std::uint32_t NN_ENGINE_ABI_STDCALL EvPeekEvent(NNEvent* outEvent)
{
	if (outEvent == nullptr)
		return 0;
	return GetQueue().Peek(*outEvent) ? 1u : 0u;
}

std::uint32_t NN_ENGINE_ABI_STDCALL EvWaitEvent(NNEvent* outEvent, std::uint32_t timeoutMs)
{
	/* Phase 2 简化：轮询方式，不阻塞。后续 Phase 可改为 condition_variable。 */
	if (outEvent == nullptr)
		return 0;
	(void)timeoutMs;
	return GetQueue().Pop(*outEvent) ? 1u : 0u;
}

std::uint32_t NN_ENGINE_ABI_STDCALL EvGetEventString(
	const NNEvent* event,
	const char** outPtr,
	std::uint16_t* outLen)
{
	if (event == nullptr || outPtr == nullptr || outLen == nullptr)
		return 0;
	return GetQueue().ReadString(event->stringPoolIdx, *outPtr, *outLen) ? 1u : 0u;
}

std::uint32_t NN_ENGINE_ABI_STDCALL EvGetQueueCount(void)
{
	return GetQueue().Size();
}

void NN_ENGINE_ABI_STDCALL EvFlushEvents(void)
{
	GetQueue().Clear();
}

std::uint32_t NN_ENGINE_ABI_STDCALL EvPushUserEvent(const NNEvent* event)
{
	if (event == nullptr)
		return 0;
	return GetQueue().Push(*event) ? 1u : 0u;
}

} // namespace

extern "C" void NNBuildEventRuntimeApi(NNEventAPI* api)
{
	if (api == nullptr)
	{
		return;
	}

	NNEventAPI built{};
	built.size = static_cast<std::uint32_t>(sizeof(NNEventAPI));
	built.pollEvent = &EvPollEvent;
	built.peekEvent = &EvPeekEvent;
	built.waitEvent = &EvWaitEvent;
	built.getEventString = &EvGetEventString;
	built.getQueueCount = &EvGetQueueCount;
	built.flushEvents = &EvFlushEvents;
	built.pushUserEvent = &EvPushUserEvent;
	*api = built;
}
