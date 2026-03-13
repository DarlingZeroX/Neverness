#pragma once
//#include "../include/CrossPlatformDefinitions.h"
#include "../imconfig.h"
#include <HCore/Include/HConfig.h>

#ifdef WIN32_NATIVE_WINDOW_SUPPORTED
#include "Win32Native/Win32Window.h"
#endif

#ifdef GLFW_WINDOW_SUPPORTED
#include "GLFW/GLFWwindow.h"
#endif

#ifdef SDL2_WINDOW_SUPPORTED
#include <HCore/Platform/SDL2/SDL2Window.h>
#endif

#ifdef SDL3_WINDOW_SUPPORTED
#include <HCorePlatform/Include/SDL3/SDL3Window.h>
#endif

#include <HCorePlatform/Interface/HWindow.h>

namespace ImGuiEx
{
	IMGUI_API void AddImGuiLayer(Horizon::HWindow& window);

	///---------------------------------
	/// Imgui
	///---------------------------------

#ifdef WIN32_NATIVE_WINDOW_SUPPORTED

	class CROSS_PLATFORM_API ImGuiWin32 :public Win32::Layer
	{
	private:
		Win32::IWin32Window* m_pWindow;
	public:
		ImGuiWin32(Win32::IWin32Window* window);
		ImGuiWin32(const ImGuiWin32&) = delete;
		ImGuiWin32& operator=(const ImGuiWin32&) = delete;
		~ImGuiWin32() override;
	public:
		static bool WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	private:
		void ImGuiInit();
		void ImGuiShutdown();
	};
#endif

#ifdef GLFW_WINDOW_SUPPORTED
	class CROSS_PLATFORM_API ImguiGLFW :public GLFWWindow::Layer
	{
	private:
		GLFWWindow::IGLFWWindow* m_pWindow;
	public:
		ImguiGLFW(GLFWWindow::IGLFWWindow* window);
		ImguiGLFW(const ImguiGLFW&) = delete;
		ImguiGLFW& operator=(const ImguiGLFW&) = delete;
		~ImguiGLFW() override;
	private:
		void ImGuiInit();
		void ImGuiShutdown();
	};
#endif

#ifdef SDL2_WINDOW_SUPPORTED
	class IMGUI_API ImguiSDL2 :public Horizon::SDL2::Layer
	{
	private:
		Horizon::SDL2::ISDL2Window* m_pWindow;
	public:
		ImguiSDL2(Horizon::SDL2::ISDL2Window* window);
		ImguiSDL2(const ImguiSDL2&) = delete;
		ImguiSDL2& operator=(const ImguiSDL2&) = delete;
		~ImguiSDL2() override;

		int ProcessMessages(const SDL_Event* event) override;
	private:
		void ImGuiInit();
		void ImGuiShutdown();
	};
#endif

#ifdef SDL3_WINDOW_SUPPORTED
	class IMGUI_API ImguiSDL3 :public Horizon::SDL3::Layer
	{
	private:
		Horizon::SDL3::ISDL3Window* m_pWindow;
	public:
		ImguiSDL3(Horizon::SDL3::ISDL3Window* window);
		ImguiSDL3(const ImguiSDL3&) = delete;
		ImguiSDL3& operator=(const ImguiSDL3&) = delete;
		~ImguiSDL3() override;

		int ProcessEvent(const SDL_Event& event) override;
	private:
		void ImGuiInit();
		void ImGuiShutdown();
	};
#endif
}