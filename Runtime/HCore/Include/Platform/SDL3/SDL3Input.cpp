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

#include "pch.h"
#include "SDL3Input.h"
#include "SDL3Window.h"
#include "SDL3KeyMap.h"

#ifdef SDL3_WINDOW_SUPPORTED
#include <SDL3\SDL_mouse.h>

namespace Horizon::SDL3
{
	//======================================================================================
	// Mouse
	//======================================================================================

	Mouse::Mouse(HMouseContextWriterInterface& context)
		:
		m_Visible(true), mMode(HRelativeMouseMode::MODE_ABSOLUTE), m_pContext(context)
	{
	}

	bool __cdecl Mouse::SetRelativeMode(HRelativeMouseMode mode)
	{
		static bool inRelativeMode = false;
		static float mouseLastX, mouseLastY;
		
		switch (mode) {
		case HRelativeMouseMode::MODE_ABSOLUTE:
		
			if (SDL_SetWindowRelativeMouseMode(m_pWindow->GetSDLWindow(), false) == true)
			{
				if (inRelativeMode)
				{
					SDL_WarpMouseGlobal(mouseLastX, mouseLastY);
					inRelativeMode = false;
				}
		
				//			H_LOG_INFO("MODE_ABSOLUTE");
				return true;
			}
		
			return false;
			break;
		case HRelativeMouseMode::MODE_RELATIVE:
			SDL_GetGlobalMouseState(&mouseLastX, &mouseLastY);
		
			if (SDL_SetWindowRelativeMouseMode(m_pWindow->GetSDLWindow(), true) == true)
			{
				inRelativeMode = true;
		
				//			H_LOG_INFO("MODE_RELATIVE");
				return true;
			}
		
			return false;
			break;
		}
		
		assert(false);
		return false;
	}

	void Mouse::WarpMouseInWindow(int x, int y)
	{
		SDL_WarpMouseInWindow(m_pWindow->GetSDLWindow(), x, y);
	}

	bool __cdecl Mouse::IsVisible() const noexcept
	{
		return m_Visible;
	}

	bool __cdecl Mouse::SetVisible(bool visible)
	{
		if (visible)
		{
			m_Visible = true;
			SDL_ShowCursor();
		}
		else
		{
			m_Visible = false;
			SDL_HideCursor();
		}

		return true;
	}

	int Mouse::ProcessEvent(const SDL_Event& evt)
	{
		const int buttonX = evt.button.x;
		const int buttonY = evt.button.y;

		const float offsetX = evt.motion.xrel;
		const float offsetY = evt.motion.yrel;

		switch (evt.type)
		{
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
			switch (evt.button.button)
			{
			case SDL_BUTTON_LEFT:
				m_pContext.OnLeftPressed();
				break;
			case SDL_BUTTON_RIGHT:
				m_pContext.OnRightPressed();

				break;
			case SDL_BUTTON_MIDDLE:
				m_pContext.OnMiddlePressed();
				break;
			default:;
			}
			break;
		case SDL_EVENT_MOUSE_BUTTON_UP:
			switch (evt.button.button)
			{
			case SDL_BUTTON_LEFT:
				m_pContext.OnLeftReleased();
				break;
			case SDL_BUTTON_RIGHT:
				m_pContext.OnRightReleased();
				break;
			case SDL_BUTTON_MIDDLE:
				m_pContext.OnMiddleReleased();
				break;
			default:;
			}
			break;

		case SDL_EVENT_MOUSE_MOTION:
			m_pContext.OnMouseMove(buttonX, buttonY);
			m_pContext.OnRawDelta(offsetX, offsetY);
			//std::cout << "OnMouseMove: " << buttonX << "," << buttonY << std::endl;
			//std::cout << "OnRawDelta: " << offsetX << "," << offsetY << std::endl;
			break;
		case SDL_EVENT_MOUSE_WHEEL:
			m_pContext.OnWheelDelta(evt.wheel.x, evt.wheel.y, evt.wheel.direction);
			break;
		default:;
		}

		return 0;
	}

	void Mouse::Update()
	{
	}

	//======================================================================================
	// Keyboard
	//======================================================================================



	int SDLKeycode2USBchar(SDL_Keycode keycode)
	{
		return static_cast<int>(SDL_KEY_MAP_USB[keycode]);
	}

	Keyboard::Keyboard(HKeyboardContextWriterInterface& context) :m_rContext(context)
	{
	}

	void Keyboard::Update()
	{
	}

	int Keyboard::ProcessEvent(const SDL_Event& evt)
	{
		const auto& key = evt.key;

		switch (evt.type)
		{
		case SDL_EVENT_KEY_DOWN:
			if (key.repeat == 0)
				m_rContext.OnKeyPressed(SDLKeycode2USBchar(key.key));
			break;
		case SDL_EVENT_KEY_UP:
			m_rContext.OnKeyReleased(SDLKeycode2USBchar(key.key));
			break;
		default: break;
		}

		return true;
	}
}

#endif