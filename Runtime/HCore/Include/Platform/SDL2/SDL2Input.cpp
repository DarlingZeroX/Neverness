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
#include "SDL2Input.h"

#include "SDL2Window.h"
#ifdef SDL2_WINDOW_SUPPORTED
#include <HCore/HLog.h>

namespace Horizon::SDL2
{
	//======================================================================================
	// Mouse
	//======================================================================================

	Mouse::Mouse(HMouseContextWriterInterface& context)
		:
		m_Visible(true), mMode(HRelativeMouseMode::MODE_ABSOLUTE), m_pContext(context)
	{}

	bool __cdecl Mouse::SetRelativeMode(HRelativeMouseMode mode)
	{
		static bool inRelativeMode = false;
		static int mouseLastX, mouseLastY;

		switch (mode) {
		case HRelativeMouseMode::MODE_ABSOLUTE:

			if (SDL_SetRelativeMouseMode(SDL_FALSE) == 0)
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

			if (SDL_SetRelativeMouseMode(SDL_TRUE) == 0)
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
			SDL_ShowCursor(SDL_ENABLE);
		}
		else
		{
			m_Visible = false;
			SDL_ShowCursor(SDL_DISABLE);
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
		case SDL_MOUSEBUTTONDOWN:
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
		case SDL_MOUSEBUTTONUP:
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

		case SDL_MOUSEMOTION:
			m_pContext.OnMouseMove(buttonX, buttonY);
			m_pContext.OnRawDelta(offsetX, offsetY);

			break;
		case SDL_MOUSEWHEEL:
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

	static std::unordered_map<int, HKeycode> SDL_KEY_MAP_USB =
	{
		{SDLK_UNKNOWN,					HKeycode::None},

		{SDLK_RETURN,					HKeycode::Enter},
		{SDLK_ESCAPE,					HKeycode::Escape},
		{SDLK_BACKSPACE,				HKeycode::Backspace},
		{SDLK_TAB,						HKeycode::Tab},
		{SDLK_SPACE,					HKeycode::Space},

		{SDLK_EXCLAIM,					HKeycode::None},
		{SDLK_QUOTEDBL,					HKeycode::None},
		{SDLK_HASH,						HKeycode::None},
		{SDLK_PERCENT,					HKeycode::None},
		{SDLK_DOLLAR,					HKeycode::None},
		{SDLK_AMPERSAND,				HKeycode::None},
		{SDLK_QUOTE,					HKeycode::None},
		{SDLK_LEFTPAREN,				HKeycode::None},
		{SDLK_RIGHTPAREN,				HKeycode::None},
		{SDLK_ASTERISK,					HKeycode::None},
		{SDLK_PLUS,						HKeycode::None},
		{SDLK_COMMA,					HKeycode::None},
		{SDLK_MINUS,					HKeycode::None},
		{SDLK_PERIOD,					HKeycode::None},
		{SDLK_SLASH,					HKeycode::None},

		{SDLK_0,						HKeycode::D0},
		{SDLK_1,						HKeycode::D1},
		{SDLK_2,						HKeycode::D2},
		{SDLK_3,						HKeycode::D3},
		{SDLK_4,						HKeycode::D4},
		{SDLK_5,						HKeycode::D5},
		{SDLK_6,						HKeycode::D6},
		{SDLK_7,						HKeycode::D7},
		{SDLK_8,						HKeycode::D8},
		{SDLK_9,						HKeycode::D9},
		{SDLK_COLON,					HKeycode::None},
		{SDLK_SEMICOLON,				HKeycode::None},
		{SDLK_LESS,						HKeycode::None},
		{SDLK_EQUALS,					HKeycode::None},
		{SDLK_GREATER,					HKeycode::None},
		{SDLK_QUESTION,					HKeycode::None},
		{SDLK_AT,						HKeycode::None},

		/*
		   Skip uppercase letters
		 */

		{SDLK_LEFTBRACKET,				HKeycode::None},
		{SDLK_BACKSLASH,				HKeycode::None},
		{SDLK_RIGHTBRACKET,				HKeycode::None},
		{SDLK_CARET,					HKeycode::None},
		{SDLK_UNDERSCORE,				HKeycode::None},
		{SDLK_BACKQUOTE,				HKeycode::None},

		{SDLK_a,						HKeycode::A},
		{SDLK_b,						HKeycode::B},
		{SDLK_c,						HKeycode::C},
		{SDLK_d,						HKeycode::D},
		{SDLK_e,						HKeycode::E},
		{SDLK_f,						HKeycode::F},
		{SDLK_g,						HKeycode::G},
		{SDLK_h,						HKeycode::H},
		{SDLK_i,						HKeycode::I},
		{SDLK_j,						HKeycode::J},
		{SDLK_k,						HKeycode::K},
		{SDLK_l,						HKeycode::L},
		{SDLK_m,						HKeycode::M},
		{SDLK_n,						HKeycode::N},
		{SDLK_o,						HKeycode::O},
		{SDLK_p,						HKeycode::P},
		{SDLK_q,						HKeycode::Q},
		{SDLK_r,						HKeycode::R},
		{SDLK_s,						HKeycode::S},
		{SDLK_t,						HKeycode::T},
		{SDLK_u,						HKeycode::U},
		{SDLK_v,						HKeycode::V},
		{SDLK_w,						HKeycode::W},
		{SDLK_x,						HKeycode::X},
		{SDLK_y,						HKeycode::Y},
		{SDLK_z,						HKeycode::Z},

		{SDLK_CAPSLOCK,					HKeycode::CapsLock},

		{SDLK_F1,						HKeycode::F1 },
		{SDLK_F2,						HKeycode::F2 },
		{SDLK_F3,						HKeycode::F3 },
		{SDLK_F4,						HKeycode::F4 },
		{SDLK_F5,						HKeycode::F5 },
		{SDLK_F6,						HKeycode::F6 },
		{SDLK_F7,						HKeycode::F7 },
		{SDLK_F8,						HKeycode::F8 },
		{SDLK_F9,						HKeycode::F9 },
		{SDLK_F10,						HKeycode::F10},
		{SDLK_F11,						HKeycode::F11},
		{SDLK_F12,						HKeycode::F12},

		{SDLK_PRINTSCREEN,				HKeycode::PrintScreen},
		{SDLK_SCROLLLOCK,				HKeycode::ScrollLock},
		{SDLK_PAUSE,					HKeycode::Pause},
		{SDLK_INSERT,					HKeycode::Insert},
		{SDLK_HOME,						HKeycode::Home},
		{SDLK_PAGEUP,					HKeycode::PageUp},
		{SDLK_DELETE,					HKeycode::Delete},
		{SDLK_END,						HKeycode::End},
		{SDLK_PAGEDOWN,					HKeycode::PageDown},
		{SDLK_RIGHT,					HKeycode::Right},
		{SDLK_LEFT,						HKeycode::Left},
		{SDLK_DOWN,						HKeycode::Down},
		{SDLK_UP,						HKeycode::Up},

		{SDLK_NUMLOCKCLEAR,				HKeycode::NumLock},
		{SDLK_KP_DIVIDE,				HKeycode::KeyPadDivide},
		{SDLK_KP_MULTIPLY,				HKeycode::KeyPadMultiply},
		{SDLK_KP_MINUS,					HKeycode::KeyPadMinus },
		{SDLK_KP_PLUS,					HKeycode::KeyPadPlus },
		{SDLK_KP_ENTER,					HKeycode::KeyPadEnter },
		{SDLK_KP_1,						HKeycode::KeyPad1},
		{SDLK_KP_2,						HKeycode::KeyPad2},
		{SDLK_KP_3,						HKeycode::KeyPad3},
		{SDLK_KP_4,						HKeycode::KeyPad4},
		{SDLK_KP_5,						HKeycode::KeyPad5},
		{SDLK_KP_6,						HKeycode::KeyPad6},
		{SDLK_KP_7,						HKeycode::KeyPad7},
		{SDLK_KP_8,						HKeycode::KeyPad8},
		{SDLK_KP_9,						HKeycode::KeyPad9},
		{SDLK_KP_0,						HKeycode::KeyPad0},
		{SDLK_KP_PERIOD,				HKeycode::None },

		{SDLK_APPLICATION,				HKeycode::None},
		{SDLK_POWER,					HKeycode::None},
		{SDLK_KP_EQUALS,				HKeycode::None},
		{SDLK_F13,						HKeycode::F13},
		{SDLK_F14,						HKeycode::F14},
		{SDLK_F15,						HKeycode::F15},
		{SDLK_F16,						HKeycode::F16},
		{SDLK_F17,						HKeycode::F17},
		{SDLK_F18,						HKeycode::F18},
		{SDLK_F19,						HKeycode::F19},
		{SDLK_F20,						HKeycode::F20},
		{SDLK_F21,						HKeycode::F21},
		{SDLK_F22,						HKeycode::F22},
		{SDLK_F23,						HKeycode::F23},
		{SDLK_F24,						HKeycode::F24},
		{SDLK_EXECUTE,					HKeycode::None},
		{SDLK_HELP,						HKeycode::None},
		{SDLK_MENU,						HKeycode::None},
		{SDLK_SELECT,					HKeycode::None},
		{SDLK_STOP,						HKeycode::None},
		{SDLK_AGAIN,					HKeycode::None},
		{SDLK_UNDO,						HKeycode::None},
		{SDLK_CUT,						HKeycode::None},
		{SDLK_COPY,						HKeycode::None},
		{SDLK_PASTE,					HKeycode::None},
		{SDLK_FIND,						HKeycode::None},
		{SDLK_MUTE,						HKeycode::None},
		{SDLK_VOLUMEUP,					HKeycode::None},
		{SDLK_VOLUMEDOWN,				HKeycode::None},
		{SDLK_KP_COMMA,					HKeycode::None},
		{SDLK_KP_EQUALSAS400,			HKeycode::None},

		{SDLK_ALTERASE,					HKeycode::None},
		{SDLK_SYSREQ,					HKeycode::None},
		{SDLK_CANCEL,					HKeycode::None},
		{SDLK_CLEAR,					HKeycode::None},
		{SDLK_PRIOR,					HKeycode::None},
		{SDLK_RETURN2,					HKeycode::None},
		{SDLK_SEPARATOR,				HKeycode::None},
		{SDLK_OUT,						HKeycode::None},
		{SDLK_OPER,						HKeycode::None},
		{SDLK_CLEARAGAIN,				HKeycode::None},
		{SDLK_CRSEL,					HKeycode::None},
		{SDLK_EXSEL,					HKeycode::None},

		{SDLK_KP_00,					HKeycode::None},
		{SDLK_KP_000,					HKeycode::None},
		{SDLK_THOUSANDSSEPARATOR,		HKeycode::None},

		{SDLK_DECIMALSEPARATOR,			HKeycode::None},

		{SDLK_CURRENCYUNIT,				HKeycode::None},
		{SDLK_CURRENCYSUBUNIT,			HKeycode::None},

		{SDLK_KP_LEFTPAREN,				HKeycode::None},
		{SDLK_KP_RIGHTPAREN,			HKeycode::None},
		{SDLK_KP_LEFTBRACE,				HKeycode::None},
		{SDLK_KP_RIGHTBRACE,			HKeycode::None},
		{SDLK_KP_TAB,					HKeycode::None},
		{SDLK_KP_BACKSPACE,				HKeycode::None},
		{SDLK_KP_A,						HKeycode::None},
		{SDLK_KP_B,						HKeycode::None},
		{SDLK_KP_C,						HKeycode::None},
		{SDLK_KP_D,						HKeycode::None},
		{SDLK_KP_E,						HKeycode::None},
		{SDLK_KP_F,						HKeycode::None},
		{SDLK_KP_XOR,					HKeycode::None},
		{SDLK_KP_POWER,					HKeycode::None},
		{SDLK_KP_PERCENT,				HKeycode::None},
		{SDLK_KP_LESS,					HKeycode::None},
		{SDLK_KP_GREATER,				HKeycode::None},
		{SDLK_KP_AMPERSAND,				HKeycode::None},
		{SDLK_KP_DBLAMPERSAND,			HKeycode::None},

		{SDLK_KP_VERTICALBAR,			HKeycode::None},

		{SDLK_KP_DBLVERTICALBAR,		HKeycode::None},

		{SDLK_KP_COLON,					HKeycode::None},
		{SDLK_KP_HASH,					HKeycode::None},
		{SDLK_KP_SPACE,					HKeycode::None},
		{SDLK_KP_AT,					HKeycode::None},
		{SDLK_KP_EXCLAM,				HKeycode::None},
		{SDLK_KP_MEMSTORE,				HKeycode::None},
		{SDLK_KP_MEMRECALL,				HKeycode::None},
		{SDLK_KP_MEMCLEAR,				HKeycode::None},
		{SDLK_KP_MEMADD,				HKeycode::None},
		{SDLK_KP_MEMSUBTRACT,			HKeycode::None},

		{SDLK_KP_MEMMULTIPLY,			HKeycode::None},

		{SDLK_KP_MEMDIVIDE,				HKeycode::None},
		{SDLK_KP_PLUSMINUS,				HKeycode::None},
		{SDLK_KP_CLEAR,					HKeycode::None},
		{SDLK_KP_CLEARENTRY,			HKeycode::None},
		{SDLK_KP_BINARY,				HKeycode::None},
		{SDLK_KP_OCTAL,					HKeycode::None},
		{SDLK_KP_DECIMAL,				HKeycode::None},
		{SDLK_KP_HEXADECIMAL,			HKeycode::None},

		{SDLK_LCTRL,					HKeycode::LControl},
		{SDLK_LSHIFT,					HKeycode::LShift},
		{SDLK_LALT,						HKeycode::LAlt},
		{SDLK_LGUI,						HKeycode::LWindows},
		{SDLK_RCTRL,					HKeycode::RControl },
		{SDLK_RSHIFT,					HKeycode::RShift },
		{SDLK_RALT,						HKeycode::RAlt },
		{SDLK_RGUI,						HKeycode::RWindows},

		{SDLK_MODE,						HKeycode::None},

		{SDLK_AUDIONEXT,				HKeycode::None},
		{SDLK_AUDIOPREV,				HKeycode::None},
		{SDLK_AUDIOSTOP,				HKeycode::None},
		{SDLK_AUDIOPLAY,				HKeycode::None},
		{SDLK_AUDIOMUTE,				HKeycode::None},
		{SDLK_MEDIASELECT,				HKeycode::None},
		{SDLK_WWW,						HKeycode::None},
		{SDLK_MAIL,						HKeycode::None},
		{SDLK_CALCULATOR,				HKeycode::None},
		{SDLK_COMPUTER,					HKeycode::None},
		{SDLK_AC_SEARCH,				HKeycode::None},
		{SDLK_AC_HOME,					HKeycode::None},
		{SDLK_AC_BACK,					HKeycode::None},
		{SDLK_AC_FORWARD,				HKeycode::None},
		{SDLK_AC_STOP,					HKeycode::None},
		{SDLK_AC_REFRESH,				HKeycode::None},
		{SDLK_AC_BOOKMARKS,				HKeycode::None},

		{SDLK_BRIGHTNESSDOWN,			HKeycode::None},

		{SDLK_BRIGHTNESSUP,				HKeycode::None},
		{SDLK_DISPLAYSWITCH,			HKeycode::None},
		{SDLK_KBDILLUMTOGGLE,			HKeycode::None},

		{SDLK_KBDILLUMDOWN,				HKeycode::None},
		{SDLK_KBDILLUMUP,				HKeycode::None},
		{SDLK_EJECT,					HKeycode::None},
		{SDLK_SLEEP,					HKeycode::None},
		{SDLK_APP1,						HKeycode::None},
		{SDLK_APP2,						HKeycode::None},

		{SDLK_AUDIOREWIND,				HKeycode::None},
		{SDLK_AUDIOFASTFORWARD,			HKeycode::None}
	};

	int SDLKeycode2USBchar(SDL_Keycode keycode)
	{
		return static_cast<int>(SDL_KEY_MAP_USB[keycode]);
	}

	Keyboard::Keyboard(HKeyboardContextWriterInterface& context) :m_rContext(context)
	{}

	void Keyboard::Update()
	{
	}

	int Keyboard::ProcessEvent(const SDL_Event& evt)
	{
		const auto& key = evt.key;

		switch (evt.type)
		{
		case SDL_KEYDOWN:
			if (key.repeat == 0)
				m_rContext.OnKeyPressed(SDLKeycode2USBchar(key.keysym.sym));
			break;
		case SDL_KEYUP:
			m_rContext.OnKeyReleased(SDLKeycode2USBchar(key.keysym.sym));
			break;
		default: break;
		}

		return true;
	}
}

#endif