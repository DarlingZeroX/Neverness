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
#include "../EngineConfig.h"
#include <sol/state.hpp>

namespace NN::Runtime
{
	struct VG_ENGINE_API VGLuaInterface
	{
		static int ExtractErrorLineNumber(const std::string& error_msg);

		static void Initialise(sol::state& L);
	};
}