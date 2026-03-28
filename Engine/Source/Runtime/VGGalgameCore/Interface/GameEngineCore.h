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
#include "IGameEngine.h"
#include "VGCore/Include/Core/Core.h"

namespace VisionGal::GalGame
{
	struct VG_GALGAME_CORE_API GameEngineCore
	{
		static void SetDesignSize(float2 size);
		static float2 GetDesignSize();

		static float GetSpriteYOffset(float size_y);
		static float GetSpriteXOffset(float size_x);

		static IGalGameEngine* GetCurrentEngine();
		static void SetCurrentEngine(IGalGameEngine* engine);
		//static GamePipeline* GetCurrentPipeline();
		//static void SetCurrentPipeline(GamePipeline* pipeline);
	};
}