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

#include "HCpuInfo.h"
#include <SDL3/SDL_cpuinfo.h>

namespace Horizon
{
	int HCpuInfo::GetCPUCount()
	{
		return SDL_GetNumLogicalCPUCores();
	}

	int HCpuInfo::GetCPUCacheLineSize()
	{
		return SDL_GetCPUCacheLineSize();
	}

	bool HCpuInfo::HasAltiVec()
	{
		return SDL_HasAltiVec();
	}

	bool HCpuInfo::HasMMX()
	{
		return SDL_HasMMX();
	}

	bool HCpuInfo::HasSSE()
	{
		return SDL_HasSSE();
	}

	bool HCpuInfo::HasSSE2()
	{
		return SDL_HasSSE2();
	}

	bool HCpuInfo::HasSSE3()
	{
		return SDL_HasSSE3();
	}

	bool HCpuInfo::HasSSE41()
	{
		return SDL_HasSSE41();
	}

	bool HCpuInfo::HasSSE42()
	{
		return SDL_HasSSE42();
	}

	bool HCpuInfo::HasAVX()
	{
		return SDL_HasAVX();
	}

	bool HCpuInfo::HasAVX2()
	{
		return SDL_HasAVX2();
	}

	bool HCpuInfo::HasAVX512F()
	{
		return SDL_HasAVX512F();
	}

	bool HCpuInfo::HasARMSIMD()
	{
		return SDL_HasARMSIMD();
	}

	bool HCpuInfo::HasNEON()
	{
		return SDL_HasNEON();
	}

	int HCpuInfo::GetSystemRAM()
	{
		return SDL_GetSystemRAM();
	}
}