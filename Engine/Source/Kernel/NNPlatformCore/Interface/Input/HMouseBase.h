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
#include <NNKernel/Interface/HConfig.h>

namespace Horizon
{
	enum class HRelativeMouseMode
	{
		MODE_ABSOLUTE = 0,
		MODE_RELATIVE,
	};

	enum class HMouseButtonState
	{
		UP = 0,         // Button is up
		HELD = 1,       // Button is held down
		RELEASED = 2,   // Button was just released
		PRESSED = 3,    // Buton was just pressed
	};

	enum class HMouseButton
	{
		LEFT = 0,		// Left mouse button
		RIGHT = 1,		// Right mouse button
		MIDDLE = 2,		// Middle mouse button
	};

	struct HMouseRawDelta
	{
		float x = .0f;
		float y = .0f;

		HMouseRawDelta() = default;
		HMouseRawDelta(float x) : x(x), y(x) {}
		HMouseRawDelta(float x, float y) : x(x), y(y) {}

		void operator=(float other)
		{
			x = other;
			y = other;
		}

		void operator+= (const HMouseRawDelta& other)
		{
			x += other.x;
			y += other.y;
		}
	};

	struct HMousePos
	{
		int x = 0;
		int y = 0;
	};

	struct HMouseState
	{
		bool    leftButton = false;		// Left button pressing ?
		bool    middleButton = false;	// Middle button pressing ?
		bool    rightButton = false;	// Right button pressing ?

		HMousePos positon;
		HMouseRawDelta rawDelta;

		//int     x = 0;
		//int     y = 0;
		int     scrollWheelValue = 0;
		HRelativeMouseMode   positionMode = HRelativeMouseMode::MODE_ABSOLUTE;
	};

	struct HMouseInterface
	{
		virtual ~HMouseInterface() = default;

		// Cursor visibility
		virtual bool IsVisible() const noexcept = 0;
		virtual bool SetVisible(bool visible) = 0;

		// Sets mouse mode (defaults to absolute)
		virtual bool SetRelativeMode(HRelativeMouseMode mode) = 0;

		virtual  void Update() {}
	};

	struct HMouseContextWriterInterface
	{
		virtual  ~HMouseContextWriterInterface() = default;

		virtual void OnMouseMove(int x, int y) noexcept = 0;
		virtual void OnMouseLeave() noexcept = 0;
		virtual void OnMouseEnter() noexcept = 0;
		virtual void OnRawDelta(float dx, float dy) noexcept = 0;

		virtual void OnLeftPressed() noexcept = 0;
		virtual void OnLeftReleased() noexcept = 0;
		virtual void OnMiddlePressed() noexcept = 0;
		virtual void OnMiddleReleased() noexcept = 0;
		virtual void OnRightPressed() noexcept = 0;
		virtual void OnRightReleased() noexcept = 0;

		virtual void OnWheelUp(int x, int y) noexcept = 0;
		virtual void OnWheelDown(int x, int y) noexcept = 0;
		virtual void OnWheelDelta(int x, int y, int delta) noexcept = 0;
	};
}