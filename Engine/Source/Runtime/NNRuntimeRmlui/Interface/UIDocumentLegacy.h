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
#include "../VGUIConfig.h"
#include "NNRuntimeCore/Include/Core/RuntimeCore.h"
#include "NNRuntimeCore/Include/Utils/TransitionHelper.h"
#include <RmlUi/Core.h>
#include <sol/function.hpp>

namespace NN::Runtime
{
	class VG_UI_API RmlUIDocumentLegacy : public VGEngineResource
	{
	public:
		RmlUIDocumentLegacy();
		~RmlUIDocumentLegacy() override;

		void Close();
		void AddUpdateCallback(const sol::function& callback);
		void AddTimerCallback(float interval, const sol::function& callback);
		void Update();

		Rml::ElementDocument* document = nullptr;
		bool isClosed = false;
		std::vector<sol::function> m_LuaUpdateCallbacks;

		struct TimerCallback
		{
			Ref<TransitionHelper> timer;
			sol::function callback;
			float interval = 0.0f;
			float elapsed = 0.0f;
		};
		std::vector<TimerCallback> m_LuaTimerCallbacks;
		std::vector<TimerCallback> m_LuaTimerUpdateAddedCallbacks;
	};
}


