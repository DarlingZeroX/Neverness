/**
 * @file BuildApplicationApi.cpp
 * @brief 导出 **NNApplicationAPI** 函数表（进程内 **RuntimeApplication** 单例）。
 */

#include "ApplicationApiExport.h"
#include "RuntimeApplicationInstance.h"

#include "NNNativeEngineAPI/Include/ApplicationAPI.h"
#include "NNNativeEngineAPI/Include/NativeInterop.h"

namespace
{
bool NN_ENGINE_ABI_STDCALL AppInitialize(void)
{
	return NN::Runtime::Application::GetRuntimeApplicationInstance().Initialize();
}

bool NN_ENGINE_ABI_STDCALL AppPumpEvents(void)
{
	return NN::Runtime::Application::GetRuntimeApplicationInstance().PumpEvents();
}

void NN_ENGINE_ABI_STDCALL AppShutdown(void)
{
	NN::Runtime::Application::GetRuntimeApplicationInstance().Shutdown();
}

void NN_ENGINE_ABI_STDCALL AppBeginFrame(void)
{
	NN::Runtime::Application::GetRuntimeApplicationInstance().BeginFrame();
}

void NN_ENGINE_ABI_STDCALL AppEndFrame(void)
{
	NN::Runtime::Application::GetRuntimeApplicationInstance().EndFrame();
}
} // namespace

extern "C" NNApplicationAPI BuildApplicationApi(void)
{
	NNApplicationAPI api{};
	api.size = static_cast<std::uint32_t>(sizeof(NNApplicationAPI));
	api.initialize = &AppInitialize;
	api.pumpEvents = &AppPumpEvents;
	api.shutdown = &AppShutdown;
	api.beginFrame = &AppBeginFrame;
	api.endFrame = &AppEndFrame;
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
