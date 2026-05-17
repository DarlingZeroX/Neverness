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
#include "RuntimeCore.h"
#include "Viewport.h"
#include <NNPlatformCore/Include/SDL3/SDL3Input.h>
#include <NNPlatformCore/Interface/Input/HKeyboardBase.h>
#include <NNPlatformCore/Include/SDL3/SDL3Window.h>

namespace NN::Runtime
{
	struct NN_RUNTIME_CORE_API Input: public NN::Core::SDL3::Layer
	{
		Input();

		static bool GetMouseButtonDown(int button);
		static bool GetMouseButtonUp(int button);
		static bool GetMouseButtonHeld(int button);

		static bool GetKey(NN::Core::HKeycode code);
		static bool GetKeyDown(NN::Core::HKeycode code);
		static bool GetKeyUp(NN::Core::HKeycode code);

		static bool GetKeyName(const String& name);
		static bool GetKeyNameDown(const String& name);
		static bool GetKeyNameUp(const String& name);

		static Input* Get();

		int ProcessEvent(const SDL_Event& event) override;
		void AttachWindow(NN::Core::SDL3::Window* window);
		void AttachViewport(Viewport* viewport);

		static NN::Core::HMouse& GetMouse();
		static NN::Core::HKeyboard& GetKeyboard();

		void Update();
	private:
		Ref<NN::Core::SDL3::Mouse> m_Mouse = nullptr;
		Ref<NN::Core::SDL3::Keyboard> m_Keyboard = nullptr;

		Viewport* m_Viewport = nullptr;
		std::unordered_map<std::string, NN::Core::HKeycode> m_NameKeycodeMap;
	};

}
