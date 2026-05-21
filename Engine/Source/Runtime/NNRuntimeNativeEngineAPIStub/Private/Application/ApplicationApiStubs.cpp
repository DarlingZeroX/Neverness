/**
 * @file ApplicationApiStubs.cpp
 * @brief **NNApplicationAPI** Stub：无 SDL；用于 Editor 测试与默认表。
 */

#include "Common/StubInvokeCounter.h"
#include "Internal/ApiStubBuilders.h"

#include "NNNativeEngineAPI/Include/ApplicationAPI.h"
#include "NNNativeEngineAPI/Include/NativeInterop.h"

namespace
{
bool NN_ENGINE_ABI_STDCALL stub_application_initialize(void)
{
	NN::StubRuntime::BumpInvokeCount();
	return false;
}

bool NN_ENGINE_ABI_STDCALL stub_application_pumpEvents(void)
{
	NN::StubRuntime::BumpInvokeCount();
	return false;
}

void NN_ENGINE_ABI_STDCALL stub_application_shutdown(void)
{
	NN::StubRuntime::BumpInvokeCount();
}

void NN_ENGINE_ABI_STDCALL stub_application_beginFrame(void)
{
	NN::StubRuntime::BumpInvokeCount();
}

void NN_ENGINE_ABI_STDCALL stub_application_endFrame(void)
{
	NN::StubRuntime::BumpInvokeCount();
}
} // namespace

extern "C" void NNBuildApplicationApiStubs(NNApplicationAPI* api)
{
	if (api == nullptr)
	{
		return;
	}

	api->size = static_cast<std::uint32_t>(sizeof(NNApplicationAPI));
	api->initialize = &stub_application_initialize;
	api->pumpEvents = &stub_application_pumpEvents;
	api->shutdown = &stub_application_shutdown;
	api->beginFrame = &stub_application_beginFrame;
	api->endFrame = &stub_application_endFrame;
}
