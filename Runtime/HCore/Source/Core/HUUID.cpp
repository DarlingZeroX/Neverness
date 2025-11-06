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

#include "pch.h"
#include "Core/HUUID.h"
#include "Core/HRandom.h"

namespace Horizon
{
	static HRandom s_random(std::clock());


	HUUID::HUUID()
		:UUID(0)
	{
		s_random.Seed(std::clock());
	}

	HUUID::HUUID(uint64 uuid)
		:UUID(uuid)
	{
	}

	HUUID HUUID::NewUUID()
	{
		s_random.Seed(std::clock());
		return HUUID(s_random.NextUINT64());
	}

	void HUUID::Invalid()
	{
		UUID = 0;
	}
}
