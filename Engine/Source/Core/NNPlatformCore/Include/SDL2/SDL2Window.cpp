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


#include "SDL2Window.h"
#ifdef SDL2_WINDOW_SUPPORTED
//#include "../../interface/HInput.h"
#include <System/HInput.h>

#pragma warning(push)
#pragma warning(disable : 26819)
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_assert.h>
#pragma warning(pop)

#include <NNCore/HLog.h>

namespace NN::Core::SDL2
{
	Window::Window(int width, int height)
	{
		m_Mouse = NewRef<Mouse>(HNativeMouse::GetContext());

		if (HNativeMouse::Get().VaildAttachedMouse() == false)
		{
			HNativeMouse::Get().MouseAttached(m_Mouse);
		}

		m_Keyboard = NewRef<Keyboard>(HNativeKeyboard::GetContext());

		if (HNativeKeyboard::Get().VaildAttachedKeyboard() == false)
		{
			HNativeKeyboard::Get().KeyboardAttached(m_Keyboard);
		}

		Initialize(width, height);
	}

	Window::~Window()
	{
		if (m_pWindow != nullptr)
		{
			SDL_DestroyWindow(m_pWindow);
		}
		m_pWindow = nullptr;
	}

	void Window::Initialize(int width, int height)
	{
		//int rendererFlags = SDL_RENDERER_ACCELERATED;

		int windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS;

		if (SDL_Init(SDL_INIT_VIDEO) < 0)
		{
			H_LOG_ERROR("Couldn't initialize SDL: {0:s}\n", SDL_GetError());
			exit(1);
		}

		m_pWindow = SDL_CreateWindow(
			"Horizon Engine", 
			SDL_WINDOWPOS_UNDEFINED, 
			SDL_WINDOWPOS_UNDEFINED, 
			width, height, 
			windowFlags
		);

		if (!m_pWindow)
		{
			H_LOG_ERROR("Failed to open {0:d} x {1:d} window: {2:}\n", width, height, SDL_GetError());
			exit(1);
		}

		SDL_SetWindowResizable(m_pWindow, SDL_TRUE);
		SDL_SetWindowHitTest(m_pWindow, HitTestCallback, nullptr);

		//SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
		//
		//m_pRenderer = SDL_CreateRenderer(app.window, -1, rendererFlags);
		//
		//if (!m_pRenderer)
		//{
		//	printf("Failed to create renderer: %s\n", SDL_GetError());
		//	exit(1);
		//}

		m_Mouse->SetWindow(this);
	}

	SDL_HitTestResult Window::HitTestCallback(SDL_Window* win, const SDL_Point* pt, void* data)
	{
#define REPORT_RESIZE_HIT(name) { \
            SDL_Log("HIT-TEST: RESIZE_" #name "\n"); \
            return SDL_HITTEST_RESIZE_##name; \
        }

		static constexpr int RESIZE_BORDER = 5;

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

	void* Window::GetNativeWindow() noexcept
	{
		SDL_SysWMinfo wmInfo;
		SDL_VERSION(&wmInfo.version);
		SDL_GetWindowWMInfo(m_pWindow, &wmInfo);

		return wmInfo.info.win.window;
	}

	EWindow Window::GetWindowType() const noexcept
	{
		return EWindow::SDL2;
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

	uint2 Window::GetWindowPos() const
	{
		int x, y;
		SDL_GetWindowPosition(m_pWindow, &x, &y);
		return { x, y };
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
		event.type = SDL_QUIT;

		SDL_PushEvent(&event);
		//SDL_Quit();
	}

	int Window::ProcessMessages() noexcept
	{
		SDL_Event event;
		//std::string dropped_filedir;

		Events::HWindowEvent windowEvent;

		while (SDL_PollEvent(&event))
		{
			for (auto& layer : m_Layers)
			{
				if (layer->ProcessMessages(&event) == 0)
					continue;
			}
		
			m_Mouse->ProcessEvent(event);
			m_Keyboard->ProcessEvent(event);
		
			switch (event.type)
			{
			case SDL_QUIT:
				return false;
			case SDL_WINDOWEVENT:
				if (event.window.windowID != SDL_GetWindowID(m_pWindow))
					break;
				windowEvent.event = static_cast<Events::HWindowEventType>(event.window.event);
				windowEvent.data1 = event.window.data1;
				windowEvent.data2 = event.window.data2;
				windowEvent.windowID = event.window.windowID;
				windowEvent.timestamp = event.window.timestamp;
		
				for (auto& listener : m_WindowEventListeners)
				{
					listener(windowEvent);
				}
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
				break;
			case SDL_DROPFILE:       // In case if dropped file
			case SDL_DROPTEXT:
				switch (event.type)
				{
				case SDL_DROPFILE:       // In case if dropped file
					windowEvent.event = Events::HWindowEventType::DROP_FILE;
					break;
				case SDL_DROPTEXT:
					windowEvent.event = Events::HWindowEventType::DROP_TEXT;
					break;
				}
		
				windowEvent.file = event.drop.file;
				SDL_free(event.drop.file);
		
			case SDL_DROPBEGIN:
			case SDL_DROPCOMPLETE:
				switch (event.type)
				{
				case SDL_DROPBEGIN:
					windowEvent.event = Events::HWindowEventType::DROP_BEGIN;
					break;
				case SDL_DROPCOMPLETE:
					windowEvent.event = Events::HWindowEventType::DROP_COMPLETE;
					break;
				}
		
				for (auto& listener : m_WindowEventListeners)
				{
					listener(windowEvent);
				}
		
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

			return true;
		}
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

	void Window::AddWindowEventListener(WindowEventListener listener)
	{
		m_WindowEventListeners.push_back(listener);
	}
}

#endif