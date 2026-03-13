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
#include "Input/HNativeMouse.h"
#include "Input/HNativeKeyboard.h"

namespace Horizon
{
	struct H_CORE_PLATFORM_API HNativeKeyboard
	{
		static HKeyboard& Get();
		static HKeyboard* GetPtr();
		static HKeyboardContext& GetContext();
	};

	struct H_CORE_PLATFORM_API HNativeMouse
	{
		static HMouse& Get();
		static HMouse* GetPtr();
		static HMouseContext& GetContext();
	};

	struct H_CORE_PLATFORM_API HInput
	{
		static HMouse& Mouse();
		static HKeyboard& Keyboard();

		static void FixedUpdate();
		static void FrameUpdate();

		static void Update();
	};
}
