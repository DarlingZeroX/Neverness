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
#include "../../HCorePlatformConfig.h"
#ifdef SDL2_WINDOW_SUPPORTED

#pragma warning(push)
#pragma warning(disable : 26819)
#include <SDL2/SDL.h>
#pragma warning(pop)

#include "../../System/Input/HNativeMouse.h"
#include "../../System/Input/HNativeKeyboard.h"

namespace Horizon::SDL2
{
	class Window;

	class CROSS_PLATFORM_API Mouse : public HMouseInterface
	{
	public:
		Mouse(HMouseContextWriterInterface& context);
		Mouse(Mouse&& moveFrom) noexcept = delete;
		Mouse& operator= (Mouse&& moveFrom) noexcept = delete;
		Mouse(const Mouse&) = delete;
		Mouse& operator=(const Mouse&) = delete;
		~Mouse() override = default;

		// Sets mouse mode (defaults to absolute)
		bool __cdecl SetRelativeMode(HRelativeMouseMode mode) override;

		void WarpMouseInWindow(int x, int y);

		// Cursor visibility
		bool __cdecl IsVisible() const noexcept override;
		bool __cdecl SetVisible(bool visible) override;

		void SetWindow(Window* window) { m_pWindow = window; }

		virtual int ProcessEvent(const SDL_Event& event);

		void Update() override;
	private:
		HMouseContextWriterInterface& m_pContext;

		HRelativeMouseMode mMode;
		bool m_Visible;
		Window* m_pWindow;
		float lastX, lastY;
	};

	class CROSS_PLATFORM_API Keyboard : public HKeyboardInterface
	{
	public:
		Keyboard(HKeyboardContextWriterInterface& context);;
		~Keyboard() override = default;

		Keyboard(Keyboard&& moveFrom) noexcept = delete;
		Keyboard& operator= (Keyboard&& moveFrom) noexcept = delete;

		Keyboard(const Keyboard&) = delete;
		Keyboard& operator=(const Keyboard&) = delete;

	public:
		void Update() override;
		virtual int ProcessEvent(const SDL_Event& event);
	private:
		HKeyboardContextWriterInterface& m_rContext;
	};
}

#endif
