/**
 * @file RuntimeApplication.cpp
 * @brief SDL3 最小 Application 宿主：Init、主窗口、事件泵、Shutdown。
 */

#include "NNRuntimeApplication/RuntimeApplication.h"

#include <SDL3/SDL.h>

namespace NN::Runtime::Application
{

bool RuntimeApplication::Initialize()
{
	if (m_sdlInitialized)
	{
		return true;
	}

	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		return false;
	}

	m_sdlInitialized = true;
	m_shouldQuit = false;
	return true;
}

bool RuntimeApplication::OpenWindow(const char* title, int width, int height)
{
	if (!m_sdlInitialized)
	{
		return false;
	}

	if (m_window != nullptr)
	{
		SDL_DestroyWindow(m_window);
		m_window = nullptr;
	}

	const char* windowTitle = (title != nullptr && title[0] != '\0') ? title : "Neverness";
	const int w = width > 0 ? width : 1280;
	const int h = height > 0 ? height : 720;

	m_window = SDL_CreateWindow(windowTitle, w, h, SDL_WINDOW_RESIZABLE);
	if (m_window == nullptr)
	{
		return false;
	}

	SDL_ShowWindow(m_window);
	m_shouldQuit = false;
	return true;
}

bool RuntimeApplication::PumpEvents()
{
	if (!m_sdlInitialized)
	{
		return false;
	}

	if (m_shouldQuit)
	{
		return false;
	}

	SDL_Event event{};
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_EVENT_QUIT)
		{
			m_shouldQuit = true;
			return false;
		}
	}

	return !m_shouldQuit;
}

void RuntimeApplication::Shutdown()
{
	if (m_window != nullptr)
	{
		SDL_DestroyWindow(m_window);
		m_window = nullptr;
	}

	if (m_sdlInitialized)
	{
		SDL_Quit();
		m_sdlInitialized = false;
	}

	m_shouldQuit = false;
}

} // namespace NN::Runtime::Application
