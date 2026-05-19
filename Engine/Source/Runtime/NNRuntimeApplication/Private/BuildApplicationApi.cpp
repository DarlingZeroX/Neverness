/**
 * @file BuildApplicationApi.cpp
 * @brief 导出 **NNApplicationAPI** 函数表（进程内 **RuntimeApplication** 单例）。
 */

#include "NNRuntimeApplication/ApplicationApiExport.h"
#include "NNRuntimeApplication/RuntimeApplication.h"

#include "NNNativeEngineAPI/Include/ApplicationAPI.h"
#include "NNNativeEngineAPI/Include/NativeInterop.h"

namespace
{
NN::Runtime::Application::RuntimeApplication GApplication;

bool NN_ENGINE_ABI_STDCALL AppInitialize(void)
{
	return GApplication.Initialize();
}

bool NN_ENGINE_ABI_STDCALL AppOpenWindow(const char* title, int width, int height)
{
	return GApplication.OpenWindow(title, width, height);
}

bool NN_ENGINE_ABI_STDCALL AppPumpEvents(void)
{
	return GApplication.PumpEvents();
}

void NN_ENGINE_ABI_STDCALL AppShutdown(void)
{
	GApplication.Shutdown();
}
} // namespace

extern "C" NNApplicationAPI BuildApplicationApi(void)
{
	NNApplicationAPI api{};
	api.size = static_cast<std::uint32_t>(sizeof(NNApplicationAPI));
	api.initialize = &AppInitialize;
	api.openWindow = &AppOpenWindow;
	api.pumpEvents = &AppPumpEvents;
	api.shutdown = &AppShutdown;
	return api;
}

extern "C" void NNBuildApplicationRuntimeApi(NNApplicationAPI* api)
{
	if (api == nullptr)
	{
		return;
	}
	*api = BuildApplicationApi();
}
