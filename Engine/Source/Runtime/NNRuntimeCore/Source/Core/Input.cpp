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
/*
#include <NNPlatformCore/Interface/HInput.h>

#include "NNCore/Interface/HStringTools.h"

namespace NN::Runtime
{
	Input::Input()
	{
		m_Mouse = MakeRef<NN::Core::SDL3::Mouse>(NN::Core::HNativeMouse::GetContext());

		if (NN::Core::HNativeMouse::Get().VaildAttachedMouse() == false)
		{
			NN::Core::HNativeMouse::Get().MouseAttached(m_Mouse);
		}

		m_Keyboard = MakeRef<NN::Core::SDL3::Keyboard>(NN::Core::HNativeKeyboard::GetContext());

		if (NN::Core::HNativeKeyboard::Get().VaildAttachedKeyboard() == false)
		{
			NN::Core::HNativeKeyboard::Get().KeyboardAttached(m_Keyboard);
		}

		//m_Mouse->SetWindow(this);

		for (auto& pair: NN::Core::HKeycodeUtils::GetKeyNameMap())
		{
			auto name = pair.second;
			NN::Core::HStringTools::ToUpper(name);
			m_NameKeycodeMap[name] = pair.first;
		}
	}

	bool Input::GetMouseButtonDown(int button)
	{
		if (Get()->m_Viewport == nullptr)
			return false;

		if (Get()->m_Viewport->IsEnableInput() == false)
			return false;
			//return NN::Core::SDL3::WINDOW_LAYER_RESULT_DEFAULT;

		return GetMouse().IsMouseButtonDown(static_cast<NN::Core::HMouseButton>(button));
	}

	bool Input::GetMouseButtonUp(int button)
	{
		if (Get()->m_Viewport == nullptr)
			return false;

		if (Get()->m_Viewport->IsEnableInput() == false)
			return false;

		return GetMouse().IsMouseButtonUp(static_cast<NN::Core::HMouseButton>(button));
	}

	bool Input::GetMouseButtonHeld(int button)
	{
		if (Get()->m_Viewport == nullptr)
			return false;

		if (Get()->m_Viewport->IsEnableInput() == false)
			return false;

		return GetMouse().IsMouseButtonHeld(static_cast<NN::Core::HMouseButton>(button));
	}

	bool Input::GetKey(NN::Core::HKeycode code)
	{
		if (Get()->m_Viewport == nullptr)
			return false;

		if (Get()->m_Viewport->IsEnableInput() == false)
			return false;

		return GetKeyboard().IsKeyHeld(code);
	}

	bool Input::GetKeyDown(NN::Core::HKeycode code)
	{
		if (Get()->m_Viewport == nullptr)
			return false;

		if (Get()->m_Viewport->IsEnableInput() == false)
			return false;

		return GetKeyboard().IsKeyDown(code);
	}

	bool Input::GetKeyUp(NN::Core::HKeycode code)
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
		NN::Core::HStringTools::ToUpper(name);

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
		NN::Core::HStringTools::ToUpper(name);

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
		NN::Core::HStringTools::ToUpper(name);

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
				return NN::Core::SDL3::WINDOW_LAYER_RESULT_DEFAULT;

			if (m_Viewport->IsEnableInput() == false)
				return NN::Core::SDL3::WINDOW_LAYER_RESULT_DEFAULT;
		}

		if (m_Mouse)
			m_Mouse->ProcessEvent(event);

		if (m_Keyboard)
			m_Keyboard->ProcessEvent(event);

		return NN::Core::SDL3::WINDOW_LAYER_RESULT_DEFAULT;
	}

	void Input::AttachWindow(NN::Core::SDL3::Window* window)
	{
		window->AddLayer(this);
	}

	void Input::AttachViewport(Viewport* viewport)
	{
		m_Viewport = viewport;
	}

	NN::Core::HMouse& Input::GetMouse()
	{
		return NN::Core::HNativeMouse::Get();
	}

	NN::Core::HKeyboard& Input::GetKeyboard()
	{
		return NN::Core::HNativeKeyboard::Get();
	}

	void Input::Update()
	{
		NN::Core::HInput::FixedUpdate();
		NN::Core::HInput::FrameUpdate();
	}
}
*/
