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
#include <chrono>
#include <functional>
#include "../EngineConfig.h"

namespace VisionGal
{
	struct VG_ENGINE_API TimeHelper
	{
		static std::string FloatToTimeFormatSprintf(float secondsFloat);
	};
}
