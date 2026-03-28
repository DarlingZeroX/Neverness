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
#include "../VGGalgameScriptLuaConfig.h"
#include "VGLua/Include/sol/table.hpp"
#include "VGLua/Include/sol/state.hpp"

namespace VisionGal::GalGame
{
	struct VG_GALGAME_SCRIPT_LUA_API GalGameLuaBinding
	{
		static void Register(sol::state& state);
		static void RegisterScript(sol::state& state);
	};
}
