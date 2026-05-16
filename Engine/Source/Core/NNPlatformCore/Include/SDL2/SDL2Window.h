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

#pragma once
#include "../WindowInterface.h"
#ifdef SDL2_WINDOW_SUPPORTED

#pragma warning(push)
#pragma warning(disable : 26819)
//#include <SDL2/include/SDL_events.h>
//#include <SDL2/include/SDL_video.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_video.h>
#pragma warning(pop)

#include "SDL2Input.h"
#include "../../CoreModuleDefinitions.h"
#include "../../Event/HWindowEvents.h"

namespace NN::Core::SDL2
{
	struct Layer
	{
		virtual ~Layer() = default;
		virtual int ProcessMessages(const SDL_Event* event) = 0;
	};

	struct ISDL2Window :public IWindow
	{
		~ISDL2Window() override = default;

		virtual SDL_Window* GetSDLWindow() noexcept = 0;
		virtual void AddLayer(std::unique_ptr<Layer>&& layer) noexcept = 0;
	};

	class H_CORE_PLATFORM_API Window final : public ISDL2Window
	{
	public:
		Window(int width, int height);
		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;
		~Window() override;
	private:
		void Initialize(int width, int height);
		static SDL_HitTestResult HitTestCallback(SDL_Window* win, const SDL_Point* area, void* data);
	public:
		std::string GetTitle() const noexcept override;
		void SetTitle(const std::string& title) override;

		virtual SDL_Window* GetSDLWindow() noexcept override;
		void* GetNativeWindow() noexcept override;
		EWindow GetWindowType() const noexcept override;

		unsigned WindowWidth() const noexcept override;
		unsigned WindowHeight() const noexcept override;

		void SetWindowPos(int x, int y) override;
		uint2 GetWindowPos() const override;

		void MinimizeWindow() override;
		void MaximizeWindow() override;
		void RestoreWindow() override;
		void SetFullscreen(bool fullscreen) override;

		void Close() override;

		int ProcessMessages() noexcept override;
		void Clear() noexcept override;
		void AddLayer(std::unique_ptr<Layer>&& layer) noexcept override;

		using WindowEventListener = std::function<void(const Events::HWindowEvent&)>;

		void AddWindowEventListener(WindowEventListener listener);
	private:
		SDL_Renderer* m_pRenderer;
		SDL_Window* m_pWindow;

		std::vector<WindowEventListener> m_WindowEventListeners;

		std::vector<std::unique_ptr<Layer>> m_Layers;

		Ref<Mouse> m_Mouse;
		Ref<Keyboard> m_Keyboard;
		//Mouse m_Mouse;
		//Keyboard m_Keyboard;
	};
}

#endif
