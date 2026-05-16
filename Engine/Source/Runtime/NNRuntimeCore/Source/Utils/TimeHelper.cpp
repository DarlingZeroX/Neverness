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

#include "Utils/TimeHelper.h"

namespace NN::Runtime
{
	std::string TimeHelper::FloatToTimeFormatSprintf(float secondsFloat)
	{
		int totalSeconds = static_cast<int>(secondsFloat + 0.5f);

		int hours = totalSeconds / 3600;
		int minutes = (totalSeconds % 3600) / 60;
		int seconds = totalSeconds % 60;

		char buffer[9]; // HH:MM:SS + null terminator
		std::sprintf(buffer, "%02d:%02d:%02d", hours, minutes, seconds);

		return std::string(buffer);
	}
}
