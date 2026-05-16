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

#include "Input/HNativeMouse.h"
#include <HCore/Interface/HLog.h>

namespace Horizon
{
	//======================================================================================
	// ButtonStateTracker
	//======================================================================================

#define UPDATE_BUTTON_STATE(field) field = static_cast<HMouseButtonState>( ( !!state.field ) | ( ( !!state.field ^ !!lastState.field ) << 1 ) );

	HMouseContext::StateTracker::StateTracker() noexcept
	{
		Reset();
	}

	void HMouseContext::StateTracker::Update(const HMouseState& state) noexcept
	{
		UPDATE_BUTTON_STATE(leftButton);

		assert((!state.leftButton && !lastState.leftButton) == (leftButton == HMouseButtonState::UP));
		assert((state.leftButton && lastState.leftButton) == (leftButton == HMouseButtonState::HELD));
		assert((!state.leftButton && lastState.leftButton) == (leftButton == HMouseButtonState::RELEASED));
		assert((state.leftButton && !lastState.leftButton) == (leftButton == HMouseButtonState::PRESSED));

		UPDATE_BUTTON_STATE(middleButton);
		UPDATE_BUTTON_STATE(rightButton);

		lastState = state;
	}

	void HMouseContext::StateTracker::OnLeftButton(bool leftPressed) noexcept
	{
		leftButton = static_cast<HMouseButtonState>((!!leftPressed) | ((!!leftPressed ^ !!lastState.leftButton) << 1));

		assert((!leftPressed && !lastState.leftButton) == (leftButton == HMouseButtonState::UP));
		assert((leftPressed && lastState.leftButton) == (leftButton == HMouseButtonState::HELD));
		assert((!leftPressed && lastState.leftButton) == (leftButton == HMouseButtonState::RELEASED));
		assert((leftPressed && !lastState.leftButton) == (leftButton == HMouseButtonState::PRESSED));

		lastState.leftButton = leftPressed;
	}

	void HMouseContext::StateTracker::OnMiddleButton(bool middlePressed) noexcept
	{
		middleButton = static_cast<HMouseButtonState>((!!middlePressed) | ((!!middlePressed ^ !!lastState.middleButton) << 1));

		lastState.middleButton = middlePressed;
	}

	void HMouseContext::StateTracker::OnRightButton(bool rightPressed) noexcept
	{
		rightButton = static_cast<HMouseButtonState>((!!rightPressed) | ((!!rightPressed ^ !!lastState.rightButton) << 1));

		lastState.rightButton = rightPressed;
	}

#undef UPDATE_BUTTON_STATE

	void HMouseContext::StateTracker::Reset() noexcept
	{
		memset(this, 0, sizeof(StateTracker));
	}

	HMouseState HMouseContext::StateTracker::GetLastState() const noexcept
	{
		return lastState;
	}

	//======================================================================================
	// HMouseButtonEvent
	//======================================================================================

	HMouseButtonEvent::HMouseButtonEvent(Type type, const HMouseContext& parent) noexcept
		:
		type(type),
		leftIsPressed(parent.state.leftButton),
		rightIsPressed(parent.state.rightButton),
		x(parent.state.positon.x),
		y(parent.state.positon.y)
	{}

	std::pair<int, int> HMouseButtonEvent::GetPos() const noexcept
	{
		return { x, y };
	}

	int HMouseButtonEvent::GetPosX() const noexcept
	{
		return x;
	}

	int HMouseButtonEvent::GetPosY() const noexcept
	{
		return y;
	}

	bool HMouseButtonEvent::LeftIsPressed() const noexcept
	{
		return leftIsPressed;
	}

	bool HMouseButtonEvent::RightIsPressed() const noexcept
	{
		return rightIsPressed;
	}

	//======================================================================================
	// HMouseContext
	//======================================================================================

	bool HMouseContext::ReadRawDelta(HMouseRawDelta& delta) noexcept
	{
		if (rawDeltaBuffer.empty())
		{
			delta.x = 0;
			delta.y = 0;
			return false;
		}
		const HMouseRawDelta d = rawDeltaBuffer.front();
		rawDeltaBuffer.pop();

		//std::cout << rawDeltaBuffer.size() << std::endl;

		delta.x = d.x;
		delta.y = d.y;

		return true;
		//std::cout << delta.x << "," << delta.y << std::endl;
	}

	HMousePos HMouseContext::GetPos() const noexcept
	{
		return state.positon;
	}

	HMouseRawDelta HMouseContext::ReadRawDelta() noexcept
	{
		if (rawDeltaBuffer.empty())
		{
			return HMouseRawDelta{};
		}
		const HMouseRawDelta d = rawDeltaBuffer.front();
		rawDeltaBuffer.pop();
		return d;
	}

	bool HMouseContext::IsInWindow() const noexcept
	{
		return isInWindow;
	}

	bool HMouseContext::LeftIsPressed() const noexcept
	{
		return state.leftButton;
	}

	bool HMouseContext::RightIsPressed() const noexcept
	{
		return state.rightButton;
	}

	std::optional<HMouseButtonEvent> HMouseContext::Read() noexcept
	{
		if (buffer.size() > 0u)
		{
			HMouseButtonEvent e = buffer.front();
			buffer.pop();
			return e;
		}
		return {};
	}

	bool HMouseContext::IsEmpty() const noexcept
	{
		return buffer.empty();
	}

	void HMouseContext::Flush() noexcept
	{
		buffer = std::queue<HMouseButtonEvent>();
	}

	void HMouseContext::EnableRaw() noexcept
	{
		rawEnabled = true;
	}

	void HMouseContext::DisableRaw() noexcept
	{
		rawEnabled = false;
	}

	bool HMouseContext::RawEnabled() const noexcept
	{
		return rawEnabled;
	}

	void HMouseContext::ClearRawBuffer() noexcept
	{
		std::queue<HMouseRawDelta> empty;
		rawDeltaBuffer.swap(empty);
	}

	void HMouseContext::OnMouseMove(int newx, int newy) noexcept
	{
		state.positon.x = newx;
		state.positon.y = newy;

		buffer.push(HMouseButtonEvent(HMouseButtonEvent::Type::Move, *this));
		TrimBuffer();
	}

	void HMouseContext::OnMouseLeave() noexcept
	{
		isInWindow = false;
		buffer.push(HMouseButtonEvent(HMouseButtonEvent::Type::Leave, *this));
		TrimBuffer();
	}

	void HMouseContext::OnMouseEnter() noexcept
	{
		isInWindow = true;
		buffer.push(HMouseButtonEvent(HMouseButtonEvent::Type::Enter, *this));
		TrimBuffer();
	}

	void HMouseContext::OnRawDelta(float dx, float dy) noexcept
	{
		state.rawDelta = { dx,dy };
		rawDeltaBuffer.push({ dx,dy });
		TrimBuffer();
	}

	void HMouseContext::OnLeftPressed() noexcept
	{
		state.leftButton = true;
		//stateTracker.OnLeftButton(state.leftButton);

		buffer.push(HMouseButtonEvent(HMouseButtonEvent::Type::LPress, *this));
		TrimBuffer();

	//	H_LOG_INFO("Mouse Left Pressed");
	}

	void HMouseContext::OnLeftReleased() noexcept
	{
		state.leftButton = false;
		//stateTracker.OnLeftButton(state.leftButton);

		buffer.push(HMouseButtonEvent(HMouseButtonEvent::Type::LRelease, *this));
		TrimBuffer();
	//	H_LOG_INFO("Mouse Left Released");
	}

	void HMouseContext::OnMiddlePressed() noexcept
	{
		state.middleButton = true;
		//stateTracker.OnLeftButton(state.middleButton);

		buffer.push(HMouseButtonEvent(HMouseButtonEvent::Type::MPress, *this));
		TrimBuffer();

	//	H_LOG_INFO("Mouse Middle Pressed");
	}
	void HMouseContext::OnMiddleReleased() noexcept
	{
		state.middleButton = false;
		//stateTracker.OnLeftButton(state.middleButton);

		buffer.push(HMouseButtonEvent(HMouseButtonEvent::Type::MRelease, *this));
		TrimBuffer();

	//	H_LOG_INFO("Mouse Middle Released");
	}

	void HMouseContext::OnRightPressed() noexcept
	{
		state.rightButton = true;
		//stateTracker.OnLeftButton(state.rightButton);

		buffer.push(HMouseButtonEvent(HMouseButtonEvent::Type::RPress, *this));
		TrimBuffer();

	//	H_LOG_INFO("Mouse Right Pressed");
	}

	void HMouseContext::OnRightReleased() noexcept
	{
		state.rightButton = false;
		//stateTracker.OnLeftButton(state.rightButton);

		buffer.push(HMouseButtonEvent(HMouseButtonEvent::Type::RRelease, *this));
		TrimBuffer();

	//	H_LOG_INFO("Mouse Right Released");
	}

	void HMouseContext::OnWheelUp(int x, int y) noexcept
	{
		buffer.push(HMouseButtonEvent(HMouseButtonEvent::Type::WheelUp, *this));
		TrimBuffer();
	}

	void HMouseContext::OnWheelDown(int x, int y) noexcept
	{
		buffer.push(HMouseButtonEvent(HMouseButtonEvent::Type::WheelDown, *this));
		TrimBuffer();
	}

	void HMouseContext::TrimBuffer() noexcept
	{
		while (buffer.size() > bufferSize)
		{
			buffer.pop();
		}
	}

	void HMouseContext::TrimRawInputBuffer() noexcept
	{
		while (rawDeltaBuffer.size() > bufferSize)
		{
			rawDeltaBuffer.pop();
		}
	}

	void HMouseContext::OnWheelDelta(int x, int y, int delta) noexcept
	{
		state.scrollWheelValue += delta;
		// generate events for every 120
	//	while (state.scrollWheelValue >= WHEEL_DELTA)
	//	{
	//		state.scrollWheelValue -= WHEEL_DELTA;
	//		OnWheelUp(x, y);
	//	}
	//	while (state.scrollWheelValue <= -WHEEL_DELTA)
	//	{
	//		state.scrollWheelValue += WHEEL_DELTA;
	//		OnWheelDown(x, y);
	//	}
	}

	void HMouseContext::FrameUpdate()
	{
		stateTracker.Update(state);
	}

	void HMouseContext::FixedUpdate()
	{
		//state.rawDelta = 0.0f;
		//
		//HMouseRawDelta temp = 0.0f;
		//
		//while(ReadRawDelta(temp))
		//{
		//	state.rawDelta += temp;
		//}

		//std::cout << state.rawDelta.x << "," << state.rawDelta.y << std::endl;
		ReadRawDelta(state.rawDelta);
	}

	const HMouseContext::StateTracker& HMouseContext::GetStateTracker() const noexcept
	{
		return stateTracker;
	}

	//======================================================================================
	// HMouseContext
	//======================================================================================

	//void HMouseContext::Update()
	//{
	//	state.leftButton = inputBuffer.leftIsPressed;
	//	state.middleButton = inputBuffer.middleIsPressed;
	//	state.rightButton = inputBuffer.rightIsPressed;
	//
	//	if (relativeMode == HRelativeMouseMode::MODE_ABSOLUTE)
	//	{
	//		state.x = inputBuffer.x;
	//		state.y = inputBuffer.y;
	//	}
	//	else if (relativeMode == HRelativeMouseMode::MODE_RELATIVE)
	//	{
	//		inputBuffer.ReadRawDelta(state.x, state.y);
	//	}
	//
	//	state.scrollWheelValue = inputBuffer.wheelDeltaCarry;
	//	state.positionMode = relativeMode;
	//
	//	tracker.Update(state);
	//}

	//======================================================================================
	// HMouse
	//======================================================================================

//	bool HMouse::IsVisible() const noexcept
//	{
//		return m_wAttachedMouse.lock()->IsVisible();
//	}
//
//	bool HMouse::SetVisible(bool visible)
//	{
//		return m_wAttachedMouse.lock()->SetVisible(visible);
//	}

	bool HMouse::SetRelativeMode(HRelativeMouseMode mode)
	{
		bool result = m_wAttachedMouse.lock()->SetRelativeMode(mode);

		if (result)
		{
			RelativeMode = mode;
			m_MouseContext.state.positionMode = mode;
		}

		return  result;
	}

	HRelativeMouseMode HMouse::GetRelativeMode() const noexcept
	{
		return m_MouseContext.state.positionMode;
	}

	const HMouseContext& HMouse::GetContext() const noexcept
	{
		return m_MouseContext;
	}

	bool HMouse::IsMouseButtonDown(HMouseButton bt) const
	{
		switch (bt)
		{
		case HMouseButton::LEFT:
			return 	m_MouseContext.GetStateTracker().leftButton == HMouseButtonState::PRESSED;
		case HMouseButton::MIDDLE:
			return 	m_MouseContext.GetStateTracker().middleButton == HMouseButtonState::PRESSED;
		case HMouseButton::RIGHT:
			return 	m_MouseContext.GetStateTracker().rightButton == HMouseButtonState::PRESSED;
		default:
			assert(false);
			return false;
		}
	}

	bool HMouse::IsMouseButtonUp(HMouseButton bt) const
	{
		switch (bt)
		{
		case HMouseButton::LEFT:
			return 	m_MouseContext.GetStateTracker().leftButton == HMouseButtonState::RELEASED;
		case HMouseButton::MIDDLE:
			return 	m_MouseContext.GetStateTracker().middleButton == HMouseButtonState::RELEASED;
		case HMouseButton::RIGHT:
			return 	m_MouseContext.GetStateTracker().rightButton == HMouseButtonState::RELEASED;
		default:
			assert(false);
			return false;
		}
	}

	bool HMouse::IsMouseButtonHeld(HMouseButton bt) const
	{
		switch (bt)
		{
		case HMouseButton::LEFT:
			return 	m_MouseContext.GetStateTracker().leftButton == HMouseButtonState::HELD;
		case HMouseButton::MIDDLE:
			return 	m_MouseContext.GetStateTracker().middleButton == HMouseButtonState::HELD;
		case HMouseButton::RIGHT:
			return 	m_MouseContext.GetStateTracker().rightButton == HMouseButtonState::HELD;
		default:
			assert(false);
			return false;
		}
	}

	HMouseRawDelta HMouse::GetRawDelta() const noexcept
	{
		return m_MouseContext.state.rawDelta;
	}

	HMousePos HMouse::GetMousePosition() const noexcept
	{

		return m_MouseContext.GetPos();
	}

	void HMouse::FixedUpdate()
	{
		m_MouseContext.FixedUpdate();
	}

	void HMouse::FrameUpdate()
	{
		m_MouseContext.FrameUpdate();
		m_wAttachedMouse.lock()->Update();
	}

	HMouse::HMouse()
		:
		Visible(
			[this]() { return m_wAttachedMouse.lock()->IsVisible(); },
			[this](bool visible) { m_wAttachedMouse.lock()->SetVisible(visible); }
		),
		RelativeMode(
			[this]() { return m_MouseContext.state.positionMode; },
			[this](HRelativeMouseMode mode) {
				if (m_wAttachedMouse.lock()->SetRelativeMode(mode))
					m_MouseContext.state.positionMode = mode;
			}
		)
	{
	}

	std::shared_ptr<HMouseInterface> HMouse::GetAttachedMouse()
	{
		return m_wAttachedMouse.lock();
	}

	bool HMouse::VaildAttachedMouse()
	{
		return !m_wAttachedMouse.expired();
	}

	void HMouse::MouseAttached(std::shared_ptr<HMouseInterface> baseMouse)
	{
		if (baseMouse == nullptr)
		{
			//assert(!m_attachedCamera.expired());
			m_wAttachedMouse.reset();
		}
		else
		{
			//assert(m_attachedCamera.expired());
			m_wAttachedMouse = baseMouse;
		}
	}
}