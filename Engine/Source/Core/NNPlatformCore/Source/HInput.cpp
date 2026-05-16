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

//
#include "HInput.h"

namespace NN::Core
{
	//======================================================================================
	// HNativeKeyboard
	//======================================================================================
	static HKeyboard keyboard;

	HKeyboard& HNativeKeyboard::Get()
	{
		return keyboard;
	}

	HKeyboard* HNativeKeyboard::GetPtr()
	{
		return &keyboard;
	}

	HKeyboardContext& HNativeKeyboard::GetContext()
	{
		return keyboard.GetContext();
	}

	//======================================================================================
	// HNativeMouse
	//======================================================================================
	static HMouse mouse;

	HMouse& HNativeMouse::Get()
	{
		return mouse;
	}

	HMouse* HNativeMouse::GetPtr()
	{
		return &mouse;
	}

	HMouseContext& HNativeMouse::GetContext()
	{
		return const_cast<HMouseContext&>(mouse.GetContext());
	}

	HMouse& HInput::Mouse()
	{
		return HNativeMouse::Get();
	}

	HKeyboard& HInput::Keyboard()
	{
		return HNativeKeyboard::Get();
	}

	void HInput::FixedUpdate()
	{
		Mouse().FixedUpdate();
	}

	void HInput::FrameUpdate()
	{
		Mouse().FrameUpdate();
		Keyboard().Update();
	}

	void HInput::Update()
	{
	}
}