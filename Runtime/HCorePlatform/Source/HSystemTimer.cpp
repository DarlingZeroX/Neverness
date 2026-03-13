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

#include "HSystemTimer.h"
#include <SDL3/SDL_timer.h>

namespace Horizon
{
	uint32 HSystemTimer::GetTicks()
	{
		return SDL_GetTicks();
	}

	uint64 HSystemTimer::GetTicks64()
	{
		return SDL_GetTicks();
	}

	uint64 HSystemTimer::GetPerformanceCounter()
	{
		return SDL_GetPerformanceCounter();
	}

	uint64 HSystemTimer::GetPerformanceFrequency()
	{
		return SDL_GetPerformanceFrequency();
	}

	void HSystemTimer::Delay(uint32 ms)
	{
		SDL_Delay(ms);
	}
}