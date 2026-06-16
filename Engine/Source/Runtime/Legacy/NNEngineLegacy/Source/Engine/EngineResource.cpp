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

#include "Engine/EngineResource.h"
#include "NNRuntimeCore/Include/Core/RuntimeCore.h"

namespace NN::Runtime
{
	std::string EngineResource::GetDefaultSpriteTexturePath()
	{
		std::string path = RuntimeCore::GetEngineResourcePathVFS() + "textures/white.png";
		return path;
	}
}
