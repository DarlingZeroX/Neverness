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

#include "Core/Input.h"
#include <HCore/Include/System/HInput.h>

#include "HCore/Include/Core/HStringTools.h"

namespace VisionGal
{
	Input::Input()
	{
		m_Mouse = NewRef<Horizon::SDL3::Mouse>(Horizon::HNativeMouse::GetContext());

		if (Horizon::HNativeMouse::Get().VaildAttachedMouse() == false)
		{
			Horizon::HNativeMouse::Get().MouseAttached(m_Mouse);
		}

		m_Keyboard = NewRef<Horizon::SDL3::Keyboard>(Horizon::HNativeKeyboard::GetContext());

		if (Horizon::HNativeKeyboard::Get().VaildAttachedKeyboard() == false)
		{
			Horizon::HNativeKeyboard::Get().KeyboardAttached(m_Keyboard);
		}

		//m_Mouse->SetWindow(this);

		for (auto& pair: Horizon::HKeycodeUtils::GetKeyNameMap())
		{
			auto name = pair.second;
			Horizon::HStringTools::ToUpper(name);
			m_NameKeycodeMap[name] = pair.first;
		}
	}

	bool Input::GetMouseButtonDown(int button)
	{
		if (Get()->m_Viewport == nullptr)
			return false;

		if (Get()->m_Viewport->IsEnableInput() == false)
			return false;
			//return Horizon::SDL3::WINDOW_LAYER_RESULT_DEFAULT;

		return GetMouse().IsMouseButtonDown(static_cast<Horizon::HMouseButton>(button));
	}

	bool Input::GetMouseButtonUp(int button)
	{
		if (Get()->m_Viewport == nullptr)
			return false;

		if (Get()->m_Viewport->IsEnableInput() == false)
			return false;

		return GetMouse().IsMouseButtonUp(static_cast<Horizon::HMouseButton>(button));
	}

	bool Input::GetMouseButtonHeld(int button)
	{
		if (Get()->m_Viewport == nullptr)
			return false;

		if (Get()->m_Viewport->IsEnableInput() == false)
			return false;

		return GetMouse().IsMouseButtonHeld(static_cast<Horizon::HMouseButton>(button));
	}

	bool Input::GetKey(Horizon::HKeycode code)
	{
		if (Get()->m_Viewport == nullptr)
			return false;

		if (Get()->m_Viewport->IsEnableInput() == false)
			return false;

		return GetKeyboard().IsKeyHeld(code);
	}

	bool Input::GetKeyDown(Horizon::HKeycode code)
	{
		if (Get()->m_Viewport == nullptr)
			return false;

		if (Get()->m_Viewport->IsEnableInput() == false)
			return false;

		return GetKeyboard().IsKeyDown(code);
	}

	bool Input::GetKeyUp(Horizon::HKeycode code)
	{
		if (Get()->m_Viewport == nullptr)
			return false;

		if (Get()->m_Viewport->IsEnableInput() == false)
			return false;

		return GetKeyboard().IsKeyUp(code);
	}

	bool Input::GetKeyName(const String& code)
	{
		auto name = code;
		Horizon::HStringTools::ToUpper(name);

		auto result = Get()->m_NameKeycodeMap.find(name);
		if (result != Get()->m_NameKeycodeMap.end())
		{
			auto keycode = result->second;
			return GetKey(keycode);
		}
	}

	bool Input::GetKeyNameDown(const String& code)
	{
		auto name = code;
		Horizon::HStringTools::ToUpper(name);

		auto result = Get()->m_NameKeycodeMap.find(name);
		if (result != Get()->m_NameKeycodeMap.end())
		{
			auto keycode = result->second;
			return GetKeyDown(keycode);
		}
	}

	bool Input::GetKeyNameUp(const String& code)
	{
		auto name = code;
		Horizon::HStringTools::ToUpper(name);

		auto result = Get()->m_NameKeycodeMap.find(name);
		if (result != Get()->m_NameKeycodeMap.end())
		{
			auto keycode = result->second;
			return GetKeyUp(keycode);
		}
	}

	Input* Input::Get()
	{
		static Input input;
		return &input;
	}

	int Input::ProcessEvent(const SDL_Event& event)
	{
		if (m_Viewport)
		{
			if (m_Viewport->GetWindowID() != event.window.windowID)
				return Horizon::SDL3::WINDOW_LAYER_RESULT_DEFAULT;

			if (m_Viewport->IsEnableInput() == false)
				return Horizon::SDL3::WINDOW_LAYER_RESULT_DEFAULT;
		}

		if (m_Mouse)
			m_Mouse->ProcessEvent(event);

		if (m_Keyboard)
			m_Keyboard->ProcessEvent(event);

		return Horizon::SDL3::WINDOW_LAYER_RESULT_DEFAULT;
	}

	void Input::AttachWindow(Horizon::SDL3::Window* window)
	{
		window->AddLayer(this);
	}

	void Input::AttachViewport(Viewport* viewport)
	{
		m_Viewport = viewport;
	}

	Horizon::HMouse& Input::GetMouse()
	{
		return Horizon::HNativeMouse::Get();
	}

	Horizon::HKeyboard& Input::GetKeyboard()
	{
		return Horizon::HNativeKeyboard::Get();
	}

	void Input::Update()
	{
		Horizon::HInput::FixedUpdate();
		Horizon::HInput::FrameUpdate();
	}
}
