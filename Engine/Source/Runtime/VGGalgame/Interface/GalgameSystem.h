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
#include "../VGGalgameConfig.h"
#include <VGEngine/Include/Game/GameEngine.h>

namespace VisionGal
{
	class VG_GALGAME_API GalGameSystem
	{
	public:
		static void Initialize(CoreGameEngine& engine);
	};
}