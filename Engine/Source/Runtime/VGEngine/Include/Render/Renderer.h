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
#include "Camera.h"
#include "../EngineConfig.h"
#include "../Core/Core.h"
#include "../Scene/Scene.h"
#include "../Scene/Components.h"

namespace VisionGal 
{
	class SpriteRendererHandler
	{
	public:
		SpriteRendererHandler();
		~SpriteRendererHandler() = default;

		//void Render(std::vector<GameActor*>& sprites, ICamera* camera);

		void Render(GameActor* sprite, ICamera* camera, uint pipelineIndex);
	};

	struct FullScreenRendererHandler
	{
		void Render(FullScreenRendererComponent* renderer);
	};
}
