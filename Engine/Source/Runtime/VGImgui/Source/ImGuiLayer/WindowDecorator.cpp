#include "pch.h"
#include "ImGuiLayer/WindowDecorator.h"
#include <VGImgui/Include/imgui/imgui.h>

#ifdef WIN32_NATIVE_WINDOW_SUPPORTED
#include <VGImgui/Include/imgui/imgui_impl_win32.h>
#endif

#ifdef GLFW_WINDOW_SUPPORTED
#include <VGImgui/Include/imgui/imgui_impl_glfw.h>
#endif

#ifdef SDL2_WINDOW_SUPPORTED
#include <imgui/Include/imgui/imgui_impl_sdl2.h>
#include <SDL2/SDL_syswm.h>
#endif

#ifdef SDL3_WINDOW_SUPPORTED
#include <VGImgui/Include/imgui/imgui_impl_sdl3.h>
#endif

namespace ImGuiEx
{
	void AddImGuiLayer(Horizon::HWindow& window)
	{
		Horizon::IWindow* pWindow = window.GetWrapWindow();
#ifdef SDL2_WINDOW_SUPPORTED
		if (window.GetWindowType() == EWindow::SDL2)
		{
			auto* sdl2Window = static_cast<Horizon::SDL2::ISDL2Window*>(pWindow);

			sdl2Window->AddLayer(std::make_unique<ImGuiEx::ImguiSDL2>(sdl2Window));

			return;
		}
#endif

#ifdef SDL3_WINDOW_SUPPORTED
		if (window.GetWindowType() == EWindow::SDL3)
		{
			auto* sdl3Window = static_cast<Horizon::SDL3::ISDL3Window*>(pWindow);

			sdl3Window->AddLayer(std::make_unique<ImGuiEx::ImguiSDL3>(sdl3Window));
			return;
		}
#endif
		throw std::runtime_error("Unknow window type");

	}

#ifdef WIN32_NATIVE_WINDOW_SUPPORTED
	ImGuiWin32::~ImGuiWin32()
	{
		ImGuiShutdown();
	}

	ImGuiWin32::ImGuiWin32(Win32::IWin32Window* window)
	{
		m_pWindow = window;
		ImGuiInit();
	}

	void ImGuiWin32::ImGuiInit()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGui::StyleColorsDark();

		io.ConfigFlags |= ImGuiConfigFlags_::ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_::ImGuiConfigFlags_NavEnableKeyboard;
		//io.ConfigFlags |= ImGuiConfigFlags_::ImGuiConfigFlags_ViewportsEnable;
		io.ConfigFlags |= ImGuiConfigFlags_::ImGuiConfigFlags_NavEnableSetMousePos;

		// Init ImGui Win32 Impl
		if (ImGui_ImplWin32_Init(m_pWindow->GetNativeWindow()) == false)
		{
			throw "";
		}

		//
		//ImGuiIO& io = ImGui::GetIO(); (void)io;
		//ImGui::StyleColorsDark();
	}

	void ImGuiWin32::ImGuiShutdown()
	{
		ImGui_ImplWin32_Shutdown();
	}

	bool ImGuiWin32::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		ImGuiIO& io = ImGui::GetIO();
		POINT mPos;
		GetCursorPos(&mPos);
		ScreenToClient(hWnd, &mPos);

		//std::cout << mPos.x << "," << mPos.y << std::endl;
		ImGui::GetIO().MousePos.x = mPos.x;
		ImGui::GetIO().MousePos.y = mPos.y;

		if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
			return true;

		const auto& imio = ImGui::GetIO();

		switch (msg)
		{
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_CHAR:
			// stifle this keyboard message if imgui wants to capture
			if (imio.WantCaptureKeyboard)
			{
				return false;
			}
			break;
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MOUSEWHEEL:
			// stifle this mouse message if imgui wants to capture
			if (imio.WantCaptureMouse)
			{
				return false;
			}
			break;
		}

		return true;
	}
#endif

#ifdef GLFW_WINDOW_SUPPORTED
	ImguiGLFW::~ImguiGLFW()
	{
		ImGuiShutdown();
	}

	ImguiGLFW::ImguiGLFW(GLFWWindow::IGLFWWindow* window)
	{
		m_pWindow = window;
		ImGuiInit();

		//Init GLFW
		ImGui_ImplGlfw_InitForOther(reinterpret_cast<GLFWwindow*>(window->GetNativeWindow()), true);
	}

	void ImguiGLFW::ImGuiInit()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGui::StyleColorsDark();

		io.ConfigFlags |= ImGuiConfigFlags_::ImGuiConfigFlags_DockingEnable;
	}

	void ImguiGLFW::ImGuiShutdown()
	{
		ImGui_ImplGlfw_Shutdown();
	}
#endif

#ifdef SDL2_WINDOW_SUPPORTED
	ImguiSDL2::~ImguiSDL2()
	{
		ImGuiShutdown();
	}

	int ImguiSDL2::ProcessMessages(const SDL_Event* event)
	{
		ImGui_ImplSDL2_ProcessEvent(event);

		auto& io = ImGui::GetIO();
		if (io.WantCaptureKeyboard || io.WantCaptureMouse)
			return 0;
		else
			return -1;
	}

	ImguiSDL2::ImguiSDL2(Horizon::SDL2::ISDL2Window* window)
	{
		m_pWindow = window;
		ImGuiInit();

		//Init SDL2
		ImGui_ImplSDL2_InitForOther(window->GetSDLWindow());
	}

	void ImguiSDL2::ImGuiInit()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGui::StyleColorsDark();

		io.ConfigFlags |= ImGuiConfigFlags_::ImGuiConfigFlags_DockingEnable;
	}

	void ImguiSDL2::ImGuiShutdown()
	{
		ImGui_ImplSDL2_Shutdown();
	}
#endif

#ifdef SDL3_WINDOW_SUPPORTED
	ImguiSDL3::ImguiSDL3(Horizon::SDL3::ISDL3Window* window)
	{
		m_pWindow = window;
		ImGuiInit();

		//Init SDL2
		ImGui_ImplSDL3_InitForOther(window->GetSDLWindow());
	}

	ImguiSDL3::~ImguiSDL3()
	{
		ImGuiShutdown();
	}

	int ImguiSDL3::ProcessEvent(const SDL_Event& event)
	{
		ImGui_ImplSDL3_ProcessEvent(&event);

		auto& io = ImGui::GetIO();
		if (io.WantCaptureKeyboard || io.WantCaptureMouse)
			return Horizon::SDL3::WINDOW_LAYER_RESULT_NO_PROPAGATE;
		
		return Horizon::SDL3::WINDOW_LAYER_RESULT_DEFAULT;
	}

	void ImguiSDL3::ImGuiInit()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGui::StyleColorsDark();

		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	}

	void ImguiSDL3::ImGuiShutdown()
	{
		ImGui_ImplSDL3_Shutdown();
	}

#endif
}