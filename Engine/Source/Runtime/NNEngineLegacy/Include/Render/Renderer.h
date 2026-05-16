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
#include "../Scene/Scene.h"
#include "../Scene/Components.h"
#include "NNRuntimeCore/Include/Core/Core.h"

namespace NN::Runtime 
{
	class VG_ENGINE_API SpriteRendererHandler
	{
	public:
		SpriteRendererHandler();
		~SpriteRendererHandler() = default;

		//void Render(std::vector<GameActor*>& sprites, ICamera* camera);

		void Render(IGameActor* sprite, ICamera* camera, uint pipelineIndex);
	};

	struct VG_ENGINE_API FullScreenRendererHandler
	{
		void Render(FullScreenRendererComponent* renderer);
	};
}
