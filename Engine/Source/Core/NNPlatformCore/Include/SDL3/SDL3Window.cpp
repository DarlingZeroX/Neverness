/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * For the latest information, see https://darlingzerox.github.io/VisionGalDoc/
 * GitHub page: https://github.com/DarlingZeroX/VisionGal
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */


#include "SDL3Window.h"
#ifdef NN_KERNEL_USE_SDL3
#include "../../Interface/HInput.h"
#include <iostream>

namespace NN::Core::SDL3
{
	Window::Window()
	{
		this->AddWindowEventListener([this](const Events::HWindowEvent& evt)
			{
				OnWindowEvent.Invoke(evt);
			});
	}

	Window::Window(int width, int height)
		:Window()
	{
		Initialize(width, height);
		InitialMouseKeyboard();
	}

	void Window::InitialMouseKeyboard()
	{
		m_Mouse = MakeRef<Mouse>(HNativeMouse::GetContext());

		if (HNativeMouse::Get().VaildAttachedMouse() == false)
		{
			HNativeMouse::Get().MouseAttached(m_Mouse);
		}

		m_Keyboard = MakeRef<Keyboard>(HNativeKeyboard::GetContext());

		if (HNativeKeyboard::Get().VaildAttachedKeyboard() == false)
		{
			HNativeKeyboard::Get().KeyboardAttached(m_Keyboard);
		}

		m_Mouse->SetWindow(this);
	}

	bool OpenGLWindow::SwapWindow()
	{
		return SDL_GL_SwapWindow(GetSDLWindow());
	}

	Window::~Window()
	{
		if (m_pWindow != nullptr)
		{
			SDL_DestroyWindow(m_pWindow);
		}
		m_pWindow = nullptr;

		SDL_Quit();
	}

	const char* Window::GetGLSLVersion() noexcept
	{
		return m_OpenGLVersion.c_str();
	}

//#include <Windows.h>
	void* Window::GetNativeWindow() noexcept
	{
		auto propID = SDL_GetWindowProperties(m_pWindow);
		return SDL_GetPointerProperty(propID, SDL_PROP_WINDOW_WIN32_HWND_POINTER, 0);
	}

	void Window::Initialize(int width, int height)
	{
		// Setup SDL
		//if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD) < 0)
		//{
		//	printf("Error: SDL_Init(): %s\n", SDL_GetError());
		//	return;
		//}
		//
		//int windowFlags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_BORDERLESS;
		////SDL_WindowFlags windowFlags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
		//
		//m_pWindow = SDL_CreateWindow("Horizon Engine", width, height, windowFlags);
		//
		//if (m_pWindow == nullptr)
		//{
		//	printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
		//	exit(1);
		//	return;
		//}
		//SDL_GetWindowID(m_pWindow);
		//SDL_SetWindowResizable(m_pWindow, true);
		//SDL_SetWindowHitTest(m_pWindow, HitTestCallback, &m_OnResizeWindowMode);
		//
		//m_Mouse->SetWindow(this);

		//////////////////////////////////////
				// Setup SDL
		if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD | SDL_INIT_AUDIO))
		{
			printf("Error: SDL_Init(): %s\n", SDL_GetError());
			return;
		}

		// Decide GL+GLSL versions
		m_OpenGLVersion = "#version 300 es";
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

		// From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
		SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

		// Create window with graphics context
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

		int windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
		//SDL_WindowFlags windowFlags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

		m_pWindow = SDL_CreateWindow("GalEngine", width, height, windowFlags);

		if (m_pWindow == nullptr)
		{
			printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
			exit(1);
			return;
		}
		SDL_GetWindowID(m_pWindow);
		SDL_SetWindowResizable(m_pWindow, true);
		SDL_SetWindowHitTest(m_pWindow, HitTestCallback, &m_OnResizeWindowMode);
	}

	SDL_HitTestResult Window::HitTestCallback(SDL_Window* win, const SDL_Point* pt, void* data)
	{
#define REPORT_RESIZE_HIT(name) { /*std::cout << "HIT-TEST: RESIZE_" #name "\n" << std::endl;*/OnResizeWindowMode = true; return SDL_HITTEST_RESIZE_##name;  }

		static constexpr int RESIZE_BORDER = 5;
		bool& OnResizeWindowMode = *reinterpret_cast<bool*>(data);

		int w, h;
		SDL_GetWindowSize(win, &w, &h);

		if (pt->x < RESIZE_BORDER && pt->y < RESIZE_BORDER) {
			REPORT_RESIZE_HIT(TOPLEFT);
		}
		else if (pt->x > RESIZE_BORDER && pt->x < w - RESIZE_BORDER && pt->y < RESIZE_BORDER) {
			REPORT_RESIZE_HIT(TOP);
		}
		else if (pt->x > w - RESIZE_BORDER && pt->y < RESIZE_BORDER) {
			REPORT_RESIZE_HIT(TOPRIGHT);
		}
		else if (pt->x > w - RESIZE_BORDER && pt->y > RESIZE_BORDER && pt->y < h - RESIZE_BORDER) {
			REPORT_RESIZE_HIT(RIGHT);
		}
		else if (pt->x > w - RESIZE_BORDER && pt->y > h - RESIZE_BORDER) {
			REPORT_RESIZE_HIT(BOTTOMRIGHT);
		}
		else if (pt->x < w - RESIZE_BORDER && pt->x > RESIZE_BORDER && pt->y > h - RESIZE_BORDER) {
			REPORT_RESIZE_HIT(BOTTOM);
		}
		else if (pt->x < RESIZE_BORDER && pt->y > h - RESIZE_BORDER) {
			REPORT_RESIZE_HIT(BOTTOMLEFT);
		}
		else if (pt->x < RESIZE_BORDER && pt->y < h - RESIZE_BORDER && pt->y > RESIZE_BORDER) {
			REPORT_RESIZE_HIT(LEFT);
		}

		return SDL_HITTEST_NORMAL;
	}

	std::string Window::GetTitle() const noexcept
	{
		return 	SDL_GetWindowTitle(m_pWindow);
	}

	void Window::SetTitle(const std::string& title)
	{
		SDL_SetWindowTitle(m_pWindow, title.c_str());
	}

	SDL_Window* Window::GetSDLWindow() noexcept
	{
		return m_pWindow;
	}

	//void* Window::GetNativeWindow() noexcept
	//{
	//	SDL_GetWindoow
	//
	//	SDL_SysWMinfo wmInfo;
	//	SDL_VERSION(&wmInfo.version);
	//	SDL_GetWindowWMInfo(m_pWindow, &wmInfo);
	//	
	//
	//	return wmInfo.info.win.window;
	//}

	EWindow Window::GetWindowType() const noexcept
	{
		return EWindow::SDL3;
	}

	unsigned Window::WindowWidth() const noexcept
	{
		int w, h;
		SDL_GetWindowSize(m_pWindow, &w, &h);
		return w;
	}

	unsigned Window::WindowHeight() const noexcept
	{
		int w, h;
		SDL_GetWindowSize(m_pWindow, &w, &h);
		return h;
	}

	void Window::SetWindowPos(int x, int y)
	{
		SDL_SetWindowPosition(m_pWindow, x, y);
	}

	int2 Window::GetWindowPos() const
	{
		int x, y;
		SDL_GetWindowPosition(m_pWindow, &x, &y);
		return { x, y };
	}

	int2 Window::GetWindowSize() const
	{
		int w, h;
		SDL_GetWindowSize(m_pWindow, &w, &h);
		return { w, h };
	}

	void Window::SetWindowSize(int w, int h)
	{
		SDL_SetWindowSize(m_pWindow, w, h);
	}

	void Window::SetWindowResizable(bool resizeable)
	{
		SDL_SetWindowResizable(m_pWindow, resizeable);
	}

	void Window::ResizeOnBorderless(int w, int h)
	{
		Events::HWindowEvent windowEvent;

		windowEvent.eventType = Events::HWindowEventType::SIZE_CHANGED;
		windowEvent.data1 = w;
		windowEvent.data2 = h;
		std::cout << "W: " << w << " H: " << h << std::endl;
		windowEvent.windowID = SDL_GetWindowID(m_pWindow);

		for (auto& listener : m_WindowEventListeners)
		{
			listener(windowEvent);
		}
	}

	void Window::MinimizeWindow()
	{
		SDL_MinimizeWindow(m_pWindow);
	}

	void Window::MaximizeWindow()
	{
		SDL_MaximizeWindow(m_pWindow);
		
	}

	void Window::RestoreWindow()
	{
		SDL_RestoreWindow(m_pWindow);
	}

	void Window::SetFullscreen(bool fullscreen)
	{
		SDL_SetWindowFullscreen(m_pWindow, (fullscreen ? SDL_WINDOW_FULLSCREEN : 0));
	}

	void Window::Close()
	{
		SDL_Event event;
		event.type = SDL_EVENT_QUIT;

		SDL_PushEvent(&event);
		//SDL_Quit();
	}

	int g_mouseDownX = -1;
	int g_mouseDownY = -1;
	int g_windowInitialWidth = 0;
	int g_windowInitialHeight = 0;
	// 窗口右下角用于调整大小的区域范围（可以根据实际情况调整大小）
	const int RESIZE_HANDLE_SIZE = 20;

	int Window::ProcessEvent(const SDL_Event& event)
	{
		Events::HWindowEvent windowEvent;

		//int w, h;
		//SDL_GetWindowSize(m_pWindow, &w, &h);
		//std::cout << "W: " << w << " H: " << h << std::endl;

		for (auto& layer : m_Layers)
		{
			if (layer->ProcessEvent(event) == WINDOW_LAYER_RESULT_NO_PROPAGATE && event.type != SDL_EVENT_WINDOW_RESIZED)
				return true;
		}

		for (auto& layer: m_LayerPtr)
		{
			if (layer->ProcessEvent(event) == WINDOW_LAYER_RESULT_NO_PROPAGATE)
				return true;
		}

		// 不是本窗口事件
		if (!IsCurrentWindowEvent(event.window.windowID))
			return true;

		if (m_Mouse)
			m_Mouse->ProcessEvent(event);

		if (m_Keyboard)
			m_Keyboard->ProcessEvent(event);

		//
		switch (event.type)
		{
		//case SDL_WINDOWEVENT:
		//	if (event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(m_pWindow))
		//		return false;
		//
		//	if (event.window.windowID != SDL_GetWindowID(m_pWindow))
		//		break;
		//	windowEvent.event = static_cast<Events::HWindowEventType>(event.window.event);
		//	windowEvent.data1 = event.window.data1;
		//	windowEvent.data2 = event.window.data2;
		//	windowEvent.windowID = event.window.windowID;
		//	windowEvent.timestamp = event.window.timestamp;
		//
		//	for (auto& listener : m_WindowEventListeners)
		//	{
		//		listener(windowEvent);
		//	}
		//switch (event.window.event)
		//{
		//case SDL_WINDOWEVENT_RESIZED:
		//	//resizeWindow(m_event.window.data1, m_event.window.data2);
		//	break;
		//case SDL_WINDOWEVENT_CLOSE:
		//	if (event.window.windowID == SDL_GetWindowID(m_pWindow))
		//		return false;
		//	break;
		//case SDL_WINDOWEVENT_MINIMIZED:
		//	std::cout << "SDL_WINDOWEVENT_MINIMIZED" << std::endl;
		//	break;
		//case SDL_WINDOWEVENT_MAXIMIZED:
		//	break;
		//case SDL_WINDOWEVENT_RESTORED:
		//	break;
		//case SDL_WINDOWEVENT_ENTER:
		//	break;
		//case SDL_WINDOWEVENT_LEAVE:
		//	break;
		//case SDL_WINDOWEVENT_FOCUS_GAINED:
		//	break;
		//case SDL_WINDOWEVENT_FOCUS_LOST:
		//	break;
		//default: break;
		//}
		//	break;
		//case SDL_EVENT_WINDOW_MAXIMIZED:
		//{
		//	SDL_RestoreWindow(m_pWindow); // 禁止系统最大化
		//	SDL_Rect usable;
		//	SDL_GetDisplayUsableBounds(SDL_GetPrimaryDisplay(), &usable);
		//	SDL_SetWindowPosition(m_pWindow, usable.x, usable.y);
		//	SDL_SetWindowSize(m_pWindow, usable.w, usable.h);
		//}
		case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
			return false;
			break;
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
			// 鼠标按下事件
			if (event.button.button == SDL_BUTTON_LEFT) {
				// 这里简单判断鼠标是否在窗口右下角区域按下，用于调整大小
				int windowX, windowY;
				SDL_GetWindowPosition(m_pWindow, &windowX, &windowY);
				int windowWidth, windowHeight;
				SDL_GetWindowSize(m_pWindow, &windowWidth, &windowHeight);
				bool onRightBottom = event.button.x >= windowWidth - RESIZE_HANDLE_SIZE &&
					event.button.x <= windowWidth &&
					event.button.y >= windowHeight - RESIZE_HANDLE_SIZE &&
					event.button.y <= windowHeight;
				if (onRightBottom) {
					g_mouseDownX = event.button.x;
					g_mouseDownY = event.button.y;
					g_windowInitialWidth = windowWidth;
					g_windowInitialHeight = windowHeight;
				}
			}
			break;
		case SDL_EVENT_MOUSE_BUTTON_UP:
			// 鼠标松开事件，重置相关记录变量
			if (event.button.button == SDL_BUTTON_LEFT) {
				g_mouseDownX = -1;
				g_mouseDownY = -1;

				m_OnResizeWindowMode = false;
			}
			break;
		case SDL_EVENT_MOUSE_MOTION:
			// 鼠标移动事件，当鼠标按下且在右下角区域时改变窗口大小
			if (g_mouseDownX != -1 && g_mouseDownY != -1) {
				int windowX, windowY;
				SDL_GetWindowPosition(m_pWindow, &windowX, &windowY);
				int newWidth = g_windowInitialWidth + (event.motion.x - g_mouseDownX);
				int newHeight = g_windowInitialHeight + (event.motion.y - g_mouseDownY);
				SDL_SetWindowSize(m_pWindow, newWidth, newHeight);
			}
			break;
		case SDL_EVENT_WINDOW_RESIZED:
			if (event.window.windowID != SDL_GetWindowID(m_pWindow))
				break;
			windowEvent.eventType = Events::HWindowEventType::SIZE_CHANGED;
			windowEvent.data1 = event.window.data1;
			windowEvent.data2 = event.window.data2;
			int windowWidth, windowHeight;
			SDL_GetWindowSize(m_pWindow, &windowEvent.data1, &windowEvent.data2);
			windowEvent.windowID = event.window.windowID;
			windowEvent.timestamp = event.window.timestamp;

			for (auto& listener : m_WindowEventListeners)
			{
				listener(windowEvent);
			}
			//}
			break;
		case SDL_EVENT_DROP_FILE:       // In case if dropped file
			windowEvent.eventType = Events::HWindowEventType::DROP_FILE;
			windowEvent.file = event.drop.data;
			for (auto& listener : m_WindowEventListeners)
			{
				listener(windowEvent);
			}
			break;
		case SDL_EVENT_DROP_TEXT:
			windowEvent.eventType = Events::HWindowEventType::DROP_TEXT;
			windowEvent.file = event.drop.data;
			for (auto& listener : m_WindowEventListeners)
			{
				listener(windowEvent);
			}
			break;
		case SDL_EVENT_DROP_BEGIN:
			windowEvent.eventType = Events::HWindowEventType::DROP_BEGIN;

			break;
		case SDL_EVENT_DROP_COMPLETE:
			windowEvent.eventType = Events::HWindowEventType::DROP_COMPLETE;

			break;
		default: break;

			//if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED && event.window.windowID == SDL_GetWindowID(window))
			//{
			//	// Release all outstanding references to the swap chain's buffers before resizing.
			//	CleanupRenderTarget();
			//	g_pSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
			//	CreateRenderTarget();
			//}
		}

		return 0;
	}

	int Window::ProcessMessages() noexcept
	{
		SDL_Event event;

		while (SDL_PollEvent(&event))
		{
			ProcessEvent(event);
			return true;
		}

		return true;
	}

	bool Window::SwapWindow()
	{
		return true;
	}

	void Window::Clear() noexcept
	{
		if (m_pWindow != nullptr)
		{
			SDL_DestroyWindow(m_pWindow);
			m_pWindow = nullptr;
		}
	}

	void Window::AddLayer(std::unique_ptr<Layer>&& layer) noexcept
	{
		m_Layers.emplace_back(std::move(layer));
	}

	void Window::AddLayer(Layer* layer) noexcept
	{
		m_LayerPtr.push_back(layer);
	}

	void Window::AddWindowEventListener(WindowEventListener listener)
	{
		m_WindowEventListeners.push_back(listener);
	}

	bool Window::IsCurrentWindowEvent(unsigned int windowID)
	{
		//if (event.window.windowID != SDL_GetWindowID(GetSDLWindow()))
		//	return 0;
		if (windowID != SDL_GetWindowID(GetSDLWindow()))
			return false;
		return true;
	}

	void Window::SetSDLWindowPtr(SDL_Window* ptr)
	{
		m_pWindow = ptr;
	}
}

#endif