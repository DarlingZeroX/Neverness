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

namespace VisionGal
{
	namespace Lua
	{
		struct TransitionManagerLuaInterface
		{
			static void Initialize(sol::state& L);
		};
	}

	struct RenderLuaInterface
	{
		static void Initialize(sol::state& L);
	};
}