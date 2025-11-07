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

#include "Lua/LuaInterface.h"		
#include "Lua/CoreLuaInterface.h"
#include "Lua/SceneLuaInterface.h"
#include "Lua/RenderLuaInterface.h"

namespace VisionGal
{
	void VGLuaInterface::Initialise(sol::state& L)
	{
		CoreLuaInterface::Initialize(L);
		SceneLuaInterface::Initialize(L);
		RenderLuaInterface::Initialize(L);
	}
}


