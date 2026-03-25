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
#include "VGCore/Include/Core/CoreTypes.h"
#include "../Include/EngineConfig.h"
#include <sol/state.hpp>

namespace VisionGal
{

	struct VG_ENGINE_API CoreLua
	{
		static void Initialize();
		static sol::state* GetCoreLuaState();

		static void Update();

		static void  RegisterGlobalAPI(const std::function<void(sol::state*)>& api);
	};


}
