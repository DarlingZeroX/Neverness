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

#include "StoryScriptLuaInterface.h"
#include <sol/sol.hpp>
#include "NNEngineLegacy/Include/Lua/LuaInterface.h"
#include "NNRuntimeCore/Include/Core/EventBus.h"

namespace VisionGal::GalGame
{
	static sol::coroutine* s_StoryScriptCoroutine = nullptr;
	static std::string s_StoryScriptPath = "";

	int StoryScriptLuaInterface::Continue(ContinueType type, int number, const std::string& str)
	{
		if (s_StoryScriptCoroutine == nullptr)
			return 0;

		if (s_StoryScriptCoroutine->status() == sol::call_status::ok) {
			s_StoryScriptCoroutine = nullptr;
		}

		if (s_StoryScriptCoroutine == nullptr)
			return 0;

		try
		{
			if (s_StoryScriptCoroutine && s_StoryScriptCoroutine->lua_state()) {

				// 调用协程
				sol::protected_function_result result;
				switch (type)
				{
				case ContinueType::None:
					result = (*s_StoryScriptCoroutine)();
					break;
				case ContinueType::Number:
					result = (*s_StoryScriptCoroutine)(number);
					break;
				case ContinueType::String:
					result = (*s_StoryScriptCoroutine)(str);
					break;
				}

				if (s_StoryScriptCoroutine->error())
				{
					sol::error err = result;
					s_StoryScriptCoroutine = nullptr;
					H_LOG_ERROR(err.what());

					int lineNumber = VGLuaInterface::ExtractErrorLineNumber(err.what());

					// 事件
					LuaScriptEvent evt;
					evt.EventType = LuaScriptEventType::ScriptError;
					evt.ScriptPath = s_StoryScriptPath;
					evt.ErrorMessage = err.what();
					evt.ErrorLineNumber = lineNumber;
					EngineEventBus::Get().OnLuaScriptEvent.Invoke(evt);
				}
			}
		}
		catch (const sol::error& e) {
			H_LOG_ERROR(e.what());
			s_StoryScriptCoroutine = nullptr;

			// 事件
			LuaScriptEvent evt;
			evt.EventType = LuaScriptEventType::ScriptError;
			evt.ScriptPath = s_StoryScriptPath;
			evt.ErrorMessage = e.what();
			evt.ErrorLineNumber = VGLuaInterface::ExtractErrorLineNumber(e.what());
			EngineEventBus::Get().OnLuaScriptEvent.Invoke(evt);
		}

		return 0;
	}

	void StoryScriptLuaInterface::ResetStoryScript()
	{
		s_StoryScriptCoroutine = nullptr;
	}

	void StoryScriptLuaInterface::SetStoryScriptCoroutine(sol::coroutine* co)
	{
		s_StoryScriptCoroutine = co;
	}

	sol::coroutine* StoryScriptLuaInterface::GetStoryScriptCoroutine()
	{
		return s_StoryScriptCoroutine;
	}

	void StoryScriptLuaInterface::SetCurrentStoryScriptPath(const std::string& path)
	{
		s_StoryScriptPath = path;
	}
}

