/**
 * @file BuildImGuiBackendApi.cpp
 * @brief 导出 **NNImGuiBackendAPI** 函数表。
 * 调用 ImGuiBackendApi.cpp 中的 nn_imgui_backend_* 函数。
 * v33 新增。
 */

#include "Engine/ImGuiBackendApi.h"
#include <SDL3/SDL.h>

namespace
{
bool NN_IMGUI_BACKEND_STDCALL ImGuiBackendInitialize(void* sdlWindow, void* device, void* context, void* swapChain)
{
	return nn_imgui_backend_initialize(
		static_cast<SDL_Window*>(sdlWindow),
		device, context, swapChain);
}

void NN_IMGUI_BACKEND_STDCALL ImGuiBackendShutdown(void)
{
	nn_imgui_backend_shutdown();
}

void NN_IMGUI_BACKEND_STDCALL ImGuiBackendNewFrame(int width, int height, int preTransform)
{
	nn_imgui_backend_new_frame(width, height, preTransform);
}

void NN_IMGUI_BACKEND_STDCALL ImGuiBackendRender(void* context, void* swapChain)
{
	nn_imgui_backend_render(context, swapChain);
}

bool NN_IMGUI_BACKEND_STDCALL ImGuiBackendProcessEvent(const void* event)
{
	return nn_imgui_backend_process_event(static_cast<const SDL_Event*>(event));
}
} // namespace

extern "C" NNImGuiBackendAPI BuildImGuiBackendApi(void)
{
	NNImGuiBackendAPI api{};
	api.size = static_cast<std::uint32_t>(sizeof(NNImGuiBackendAPI));
	api.initialize = &ImGuiBackendInitialize;
	api.shutdown = &ImGuiBackendShutdown;
	api.newFrame = &ImGuiBackendNewFrame;
	api.render = &ImGuiBackendRender;
	api.processEvent = &ImGuiBackendProcessEvent;
	return api;
}

extern "C" void NNBuildImGuiBackendRuntimeApi(NNImGuiBackendAPI* api)
{
	if (api == nullptr)
	{
		return;
	}
	*api = BuildImGuiBackendApi();
}
