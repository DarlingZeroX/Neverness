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
 
#include "NNRuntimeCore/Include/Core/Core.h"
#include "NNRuntimeCore/Interface/GameInterface.h"
#include <VGGalgameCore/Interface/IGameEngine.h>

namespace VisionGal::GalGame {

	class SpriteTransformScriptManager
	{
	public:
		static SpriteTransformScriptManager* GetInstance();

		static Ref<IAnimationScript> CreateSpriteTransformWithCommand(IGalGameEngine* engine, IGameActor* actor, const String& cmd);

		static bool StartSpriteTransformWithCommand(IGalGameEngine* engine, IGameActor* actor, const String& cmd);
	};

}