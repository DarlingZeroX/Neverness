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
#pragma warning(push)
#pragma warning(disable : 26819)
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_video.h>
#pragma warning(pop)

#include "SDL3Input.h"
#include "../WindowInterface.h"
#include "../../HCorePlatformConfig.h"
#include <vector>
#include <NNCore/Interface/HVector.h>

#ifdef NN_KERNEL_USE_SDL3

namespace NN::Core::SDL3
{
	enum SDL3_WINDOW_LAYER_RESULT
	{
		WINDOW_LAYER_RESULT_DEFAULT = 0,
		WINDOW_LAYER_RESULT_NO_PROPAGATE = 1 
	};

	struct Layer
	{
		virtual ~Layer() = default;

		virtual int ProcessEvent(const SDL_Event& event) = 0;
	};

	struct ISDL3Window :public IWindow
	{
		~ISDL3Window() override = default;

		virtual SDL_Window* GetSDLWindow() noexcept = 0;
		virtual const char* GetGLSLVersion() noexcept = 0;
		virtual void AddLayer(std::unique_ptr<Layer>&& layer) noexcept = 0;
		virtual void AddLayer(Layer* layer) noexcept = 0;
	};

	class H_CORE_PLATFORM_API Window : public ISDL3Window
	{
	public:
		Window();
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

		void* GetNativeWindow() noexcept override;
		EWindow GetWindowType() const noexcept override;

		unsigned WindowWidth() const noexcept override;
		unsigned WindowHeight() const noexcept override;

		void SetWindowPos(int x, int y) override;
		int2 GetWindowPos() const override;
		int2 GetWindowSize() const override;
		void SetWindowSize(int w, int h) override;
		void SetWindowResizable(bool resizeable) override;
		void ResizeOnBorderless(int w, int h);

		void MinimizeWindow() override;
		void MaximizeWindow() override;
		void RestoreWindow() override;
		void SetFullscreen(bool fullscreen) override;

		void Close() override;

		int ProcessMessages() noexcept override;
		bool SwapWindow() override;
		int ProcessEvent(const SDL_Event& event);
		void Clear() noexcept override;
		void AddLayer(std::unique_ptr<Layer>&& layer) noexcept override;
		void AddLayer(Layer* layer) noexcept override;
		void AddWindowEventListener(WindowEventListener listener) override;
		bool IsCurrentWindowEvent(unsigned int windowID) override;

		SDL_Window* GetSDLWindow() noexcept override;
		const char* GetGLSLVersion() noexcept override;
	protected:
		void SetSDLWindowPtr(SDL_Window* ptr);
		void InitialMouseKeyboard();

	protected:
		std::string m_OpenGLVersion;
	private:
		SDL_Window* m_pWindow;

		std::vector<WindowEventListener> m_WindowEventListeners;
		std::vector<std::unique_ptr<Layer>> m_Layers;
		std::vector<Layer*> m_LayerPtr;

		Ref<Mouse> m_Mouse = nullptr;
		Ref<Keyboard> m_Keyboard = nullptr;

		bool m_OnResizeWindowMode = false;
		int2 m_RestorePos = { 0, 0 };
		int2 m_RestoreSize = { 0, 0}; 
		int2 m_LastWindowSize = {0,0};
	};

	struct H_CORE_PLATFORM_API OpenGLWindow: public Window
	{
		~OpenGLWindow() override = default;

		virtual SDL_GLContext GetContext() = 0;
		bool SwapWindow() override;
	};
}

#endif
