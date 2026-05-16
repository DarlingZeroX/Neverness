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
#include "SceneInterface.h"

namespace NN::Runtime
{
	struct VG_CORE_API EngineRuntimeState
	{
		static EngineRuntimeState& Get();

		bool isPlayMode = false;
	};
}
