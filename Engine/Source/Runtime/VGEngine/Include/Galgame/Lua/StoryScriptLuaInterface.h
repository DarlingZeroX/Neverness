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
#include <sol/state.hpp>

namespace VisionGal::GalGame
{
	struct StoryScriptLuaInterface
	{
		enum class ContinueType
		{
			None = 0,
			Number = 1,
			String = 2,
		};

		static int Continue(ContinueType type = ContinueType::None, int number = 0, const std::string& str = "");
		static void ResetStoryScript();

		static void SetStoryScriptCoroutine(sol::coroutine* co);
		static sol::coroutine* GetStoryScriptCoroutine();

		static void SetCurrentStoryScriptPath(const String& path);
		//
		//	static void Initialise(lua_State* L);
		//
		static void Initialise(sol::state& lua);
	};
}
