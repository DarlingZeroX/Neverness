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

#include "Input/HNativeKeyboard.h"
#include "HCore/Interface/HLog.h"

namespace Horizon
{
	//======================================================================================
	// HKeyboardEvent
	//======================================================================================
	HKeyboardEvent::HKeyboardEvent(Type type, unsigned char code) noexcept :
		type(type),
		code(code)
	{}

	bool HKeyboardEvent::IsPress() const noexcept
	{
		return type == Type::Press;
	}

	bool HKeyboardEvent::IsRelease() const noexcept
	{
		return type == Type::Release;
	}

	unsigned char HKeyboardEvent::GetCode() const noexcept
	{
		return code;
	}

	//======================================================================================
	// HKBState
	//======================================================================================

	void HKBState::Reset()
	{
		released.reset();
		pressed.reset();
		held.reset();
	}

	//======================================================================================
	// HKeyboardContext
	//======================================================================================

	std::optional<HKeyboardEvent> HKeyboardContext::ReadKey() noexcept
	{
		if (keybuffer.size() > 0u)
		{
			HKeyboardEvent e = keybuffer.front();
			keybuffer.pop();
			return e;
		}
		return {};
	}

	bool HKeyboardContext::KeyIsEmpty() const noexcept
	{
		return keybuffer.empty();
	}

	std::optional<char> HKeyboardContext::ReadChar() noexcept
	{
		if (charbuffer.size() > 0u)
		{
			unsigned char charcode = charbuffer.front();
			charbuffer.pop();
			return charcode;
		}
		return {};
	}

	bool HKeyboardContext::CharIsEmpty() const noexcept
	{
		return charbuffer.empty();
	}

	void HKeyboardContext::FlushKey() noexcept
	{
		keybuffer = std::queue<HKeyboardEvent>();
	}

	void HKeyboardContext::FlushChar() noexcept
	{
		charbuffer = std::queue<char>();
	}

	void HKeyboardContext::Flush() noexcept
	{
		FlushKey();
		FlushChar();
	}

	void HKeyboardContext::EnableAutorepeat() noexcept
	{
		autorepeatEnabled = true;
	}

	void HKeyboardContext::DisableAutorepeat() noexcept
	{
		autorepeatEnabled = false;
	}

	bool HKeyboardContext::AutorepeatIsEnabled() const noexcept
	{
		return autorepeatEnabled;
	}

	void HKeyboardContext::OnKeyPressed(unsigned char keycode) noexcept
	{
		keyPressed[keycode] = true;
		states.held[keycode] = true;

		keybuffer.push(HKeyboardEvent(HKeyboardEvent::Type::Press, keycode));
		TrimBuffer(keybuffer);

	//	H_LOG_INFO("On Key Pressed: {0:s}", HKeycodeUtils::GetKeyName(static_cast<HKeycode>(keycode)).c_str());
	}

	void HKeyboardContext::OnKeyReleased(unsigned char keycode) noexcept
	{
		keyReleased[keycode] = true;
		states.held[keycode] = false;

		keybuffer.push(HKeyboardEvent(HKeyboardEvent::Type::Release, keycode));
		TrimBuffer(keybuffer);

	//	H_LOG_INFO("On Key Released: {0:s}", HKeycodeUtils::GetKeyName(static_cast<HKeycode>(keycode)).c_str());
	}

	void HKeyboardContext::OnChar(char character) noexcept
	{
		charbuffer.push(character);
		TrimBuffer(charbuffer);
	}

	void HKeyboardContext::ClearState() noexcept
	{
		keyPressed.reset();
		keyReleased.reset();

		states.Reset();
	}

	void HKeyboardContext::TrimBuffer(std::queue<HKeyboardEvent>& buffer) noexcept
	{
		while (buffer.size() > bufferSize)
		{
			buffer.pop();
		}
	}

	void HKeyboardContext::TrimBuffer(std::queue<char>& buffer) noexcept
	{
		while (buffer.size() > bufferSize)
		{
			buffer.pop();
		}
	}

	void HKeyboardContext::Update()
	{
		states.pressed = keyPressed;
		states.pressed = keyReleased;

		keyPressed.reset();
		keyReleased.reset();
	}

	//======================================================================================
	// HKeyboard
	//======================================================================================

	std::shared_ptr<HKeyboardInterface> HKeyboard::GetAttachedKeyboard()
	{
		return m_wAttachedKeyboard.lock();
	}

	bool HKeyboard::VaildAttachedKeyboard()
	{
		return !m_wAttachedKeyboard.expired();
	}

	void HKeyboard::KeyboardAttached(std::shared_ptr<HKeyboardInterface> kb)
	{
		if (kb == nullptr)
		{
			//assert(!m_attachedCamera.expired());
			m_wAttachedKeyboard.reset();
		}
		else
		{
			//assert(m_attachedCamera.expired());
			m_wAttachedKeyboard = kb;
		}
	}

	//const HKBState& HKeyboard::State() const
	//{
	//	return m_KeyboardContext.states;
	//}

	void HKeyboard::Update()
	{
		m_KeyboardContext.Update();
		m_wAttachedKeyboard.lock()->Update();
	}

	bool HKeyboard::IsKeyDown(HKeycode key) const
	{
		return m_KeyboardContext.states.pressed[static_cast<char>(key)];
	}

	bool HKeyboard::IsKeyUp(HKeycode key) const
	{
		return m_KeyboardContext.states.released[static_cast<char>(key)];
	}

	bool HKeyboard::IsKeyHeld(HKeycode key) const
	{
		return m_KeyboardContext.states.held[static_cast<char>(key)];
	}

	HKeyboardContext& HKeyboard::GetContext() const noexcept
	{
		return const_cast<HKeyboardContext&>(m_KeyboardContext);
	}
}