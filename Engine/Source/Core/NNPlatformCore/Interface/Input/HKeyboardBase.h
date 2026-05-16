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
#include <bitset>
//#include "CrossPlatformDefinitions.h"
#include <unordered_map>

#include "../../HCorePlatformConfig.h"

//enum class HKBButtonState
//{
//	UP = 0,         // Button is up
//	HELD = 1,       // Button is held down
//	RELEASED = 2,   // Button was just released
//	PRESSED = 3,    // Buton was just pressed
//};
namespace NN::Core
{

	enum class HKeycode : int
	{
		None = 0,

		Escape,
		Tab,
		CapsLock,
		Space,
		Enter,
		Backspace,

		LShift,
		LControl,
		LWindows,
		LAlt,
		RShift,
		RControl,
		RWindows,
		RAlt,
		//Exclaim,		// !
		//At,				// @
		//Hash,			// #
		//Dollar,			// $
		//Percent,		// %
		//Caret,			// ^
		//Ampersand,		// &
		//Asterisk,		// *
		//LeftParen,		// (
		//RightParen,		// )

		D0 = 0x30,
		D1 = 0x31,
		D2 = 0x32,
		D3 = 0x33,
		D4 = 0x34,
		D5 = 0x35,
		D6 = 0x36,
		D7 = 0x37,
		D8 = 0x38,
		D9 = 0x39,

		A = 0x41,
		B = 0x42,
		C = 0x43,
		D = 0x44,
		E = 0x45,
		F = 0x46,
		G = 0x47,
		H = 0x48,
		I = 0x49,
		J = 0x4a,
		K = 0x4b,
		L = 0x4c,
		M = 0x4d,
		N = 0x4e,
		O = 0x4f,
		P = 0x50,
		Q = 0x51,
		R = 0x52,
		S = 0x53,
		T = 0x54,
		U = 0x55,
		V = 0x56,
		W = 0x57,
		X = 0x58,
		Y = 0x59,
		Z = 0x5a,

		F1 = 0x70,
		F2 = 0x71,
		F3 = 0x72,
		F4 = 0x73,
		F5 = 0x74,
		F6 = 0x75,
		F7 = 0x76,
		F8 = 0x77,
		F9 = 0x78,
		F10 = 0x79,
		F11 = 0x7a,
		F12 = 0x7b,
		F13 = 0x7c,
		F14 = 0x7d,
		F15 = 0x7e,
		F16 = 0x7f,
		F17 = 0x80,
		F18 = 0x81,
		F19 = 0x82,
		F20 = 0x83,
		F21 = 0x84,
		F22 = 0x85,
		F23 = 0x86,
		F24 = 0x87,

		PrintScreen,
		ScrollLock,
		Pause,
		Insert,
		Home,
		PageUp,
		Delete,
		End,
		PageDown,
		Left,
		Up,
		Right,
		Down,

		NumLock = 0x6a,
		KeyPadDivide = 0x6b,
		KeyPadMultiply = 0x6c,
		KeyPadMinus = 0x6d,
		KeyPadPlus = 0x6e,
		KeyPadEnter = 0x6f,
		KeyPad0 = 0x60,
		KeyPad1 = 0x61,
		KeyPad2 = 0x62,
		KeyPad3 = 0x63,
		KeyPad4 = 0x64,
		KeyPad5 = 0x65,
		KeyPad6 = 0x66,
		KeyPad7 = 0x67,
		KeyPad8 = 0x68,
		KeyPad9 = 0x69,
		KeyPadPeriod = 0x70,
	};

	struct H_CORE_PLATFORM_API HKeycodeUtils
	{
		static std::string GetKeyName(HKeycode code);
		static std::unordered_map<HKeycode, std::string>& GetKeyNameMap();
	};

	struct HKeyboardInterface
	{
		virtual ~HKeyboardInterface() = default;

		// Reset the keyboard state
		virtual void __cdecl Reset() {}

		virtual void Update() {}
	};

	struct HKeyboardContextWriterInterface
	{
		virtual ~HKeyboardContextWriterInterface() = default;

		virtual void OnKeyPressed(unsigned char vkcode) noexcept = 0;
		virtual void OnKeyReleased(unsigned char vkcode) noexcept = 0;
		virtual void OnChar(char character) noexcept = 0;
	};
}