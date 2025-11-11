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
#include "../Lua/sol2/sol.hpp"

namespace VisionGal::GalGame
{
	struct StoryScriptLuaInterface
	{
		static int Continue();
		static void ResetStoryScript();

		static void SetStoryScriptCoroutine(sol::coroutine* co);
		static sol::coroutine* GetStoryScriptCoroutine();

		static sol::state& GetLuaState();
		static sol::state& ResetLuaState();
		//
		//	static void Initialise(lua_State* L);
		//
		static void Initialise(sol::state& lua);
	};
}
