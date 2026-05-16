#pragma once
//#include "../include/CrossPlatformDefinitions.h"
#include "../imconfig.h"
#include <NNCore/Interface/HConfig.h>

#ifdef NN_KERNEL_USE_SDL3
#include <NNPlatformCore/Include/SDL3/SDL3Window.h>
#endif

#include <NNPlatformCore/Interface/HWindow.h>

namespace ImGuiEx
{
	IMGUI_API void AddSDL3ImGuiWindowLayer(NN::Core::HWindow& window, SDL_Renderer* renderer);

#ifdef NN_KERNEL_USE_SDL3
	class IMGUI_API SDL3RendererImGuiWindowLayer :public NN::Core::SDL3::Layer
	{
	private:
		NN::Core::SDL3::ISDL3Window* m_pWindow;
	public:
		SDL3RendererImGuiWindowLayer(NN::Core::SDL3::ISDL3Window* window, SDL_Renderer* renderer);
		SDL3RendererImGuiWindowLayer(const SDL3RendererImGuiWindowLayer&) = delete;
		SDL3RendererImGuiWindowLayer& operator=(const SDL3RendererImGuiWindowLayer&) = delete;
		~SDL3RendererImGuiWindowLayer() override;

		int ProcessEvent(const SDL_Event& event) override;
	private:
		void ImGuiInit();
		void ImGuiShutdown();
	};
#endif

	class IMGUI_API Opengl3ImGuiWindowLayer :public NN::Core::SDL3::Layer
	{
	private:
		NN::Core::SDL3::ISDL3Window* m_pWindow;
	public:
		Opengl3ImGuiWindowLayer(NN::Core::SDL3::ISDL3Window* window);
		Opengl3ImGuiWindowLayer(const Opengl3ImGuiWindowLayer&) = delete;
		Opengl3ImGuiWindowLayer& operator=(const Opengl3ImGuiWindowLayer&) = delete;
		~Opengl3ImGuiWindowLayer() override;

		int ProcessEvent(const SDL_Event& event) override;
	private:
		void ImGuiInit();
		void ImGuiShutdown();
	};
}