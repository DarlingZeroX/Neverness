#include "pch.h"
#include "ImGuiLayer/SDL3Decorator.h"
#include <VGImgui/Include/imgui/imgui.h>
#include "ImGuiEx/ImGuiEx.h"

#ifdef SDL3_WINDOW_SUPPORTED
#include <VGImgui/Include/imgui/imgui_impl_sdl3.h>
#include <VGImgui/Include/imgui/imgui_impl_sdlrenderer3.h>
#endif

namespace ImGuiEx
{
	void AddSDL3ImGuiWindowLayer(Horizon::HWindow& window, SDL_Renderer* renderer)
	{
		Horizon::IWindow* pWindow = window.GetWrapWindow();

#ifdef SDL3_WINDOW_SUPPORTED
		if (window.GetWindowType() == EWindow::SDL3)
		{
			auto* sdl3Window = static_cast<Horizon::SDL3::ISDL3Window*>(pWindow);

			sdl3Window->AddLayer(std::make_unique<ImGuiEx::SDL3RendererImGuiWindowLayer>(sdl3Window, renderer));
			return;
		}
#endif
		throw std::runtime_error("Unknow window type");

	} 

#ifdef SDL3_WINDOW_SUPPORTED
	SDL3RendererImGuiWindowLayer::SDL3RendererImGuiWindowLayer(Horizon::SDL3::ISDL3Window* window, SDL_Renderer* renderer)
	{
		m_pWindow = window;
		ImGuiInit();

		//Init SDL3
		ImGui_ImplSDL3_InitForSDLRenderer(window->GetSDLWindow(), renderer);
		ImGui_ImplSDLRenderer3_Init(renderer);
	}

	SDL3RendererImGuiWindowLayer::~SDL3RendererImGuiWindowLayer()
	{
		ImGuiShutdown();
	}

	int SDL3RendererImGuiWindowLayer::ProcessEvent(const SDL_Event& event)
	{
		ImGui_ImplSDL3_ProcessEvent(&event);

		auto& io = ImGui::GetIO();
		if (io.WantCaptureKeyboard || io.WantCaptureMouse)
			return Horizon::SDL3::WINDOW_LAYER_RESULT_NO_PROPAGATE;
		else
			return Horizon::SDL3::WINDOW_LAYER_RESULT_DEFAULT;
	}

	void SDL3RendererImGuiWindowLayer::ImGuiInit()
	{
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
//		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsLight();
	}

	void SDL3RendererImGuiWindowLayer::ImGuiShutdown()
	{
		ImGui_ImplSDL3_Shutdown();
	}

#endif

	Opengl3ImGuiWindowLayer::Opengl3ImGuiWindowLayer(Horizon::SDL3::ISDL3Window* window)
	{
		m_pWindow = window;
		ImGuiInit();
	}

	Opengl3ImGuiWindowLayer::~Opengl3ImGuiWindowLayer()
	{
		ImGuiShutdown();
	}

	int Opengl3ImGuiWindowLayer::ProcessEvent(const SDL_Event& event)
	{
		//SDL_StartTextInput
		ImGui_ImplSDL3_ProcessEvent(&event);

		auto& io = ImGui::GetIO();


		//if (io.WantCaptureKeyboard || io.WantCaptureMouse)
		//{
		//	//H_LOG_INFO("Horizon::SDL3::WINDOW_LAYER_RESULT_NO_PROPAGATE");
		//	return Horizon::SDL3::WINDOW_LAYER_RESULT_NO_PROPAGATE;
		//}
		//else
		//{
			//H_LOG_INFO("Horizon::SDL3::WINDOW_LAYER_RESULT_DEFAUL");
			return Horizon::SDL3::WINDOW_LAYER_RESULT_DEFAULT;
		//}
	}

	void Opengl3ImGuiWindowLayer::ImGuiInit()
	{
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsLight();
	}

	void Opengl3ImGuiWindowLayer::ImGuiShutdown()
	{
		ImGui_ImplSDL3_Shutdown();
	}
}