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

#include "GameEngineCore.h"
#include "VGEngine/Include/Project/ProjectSettings.h"

namespace VisionGal::GalGame
{
	struct GameCoreImp
	{
		GameCoreImp()
		{
			GameEngine = nullptr;
		}

		static GameCoreImp* Get()
		{
			static GameCoreImp imp;
			return &imp;
		}

		IGalGameEngine* GameEngine;
	};

	void GameEngineCore::SetDesignSize(float2 size)
	{
		auto& settings = ProjectSettings::GetProjectSettings().GalGame;
		settings.DesignWidth = static_cast<int>(size.x);
		settings.DesignHeight = static_cast<int>(size.y);
	}

	float2 GameEngineCore::GetDesignSize()
	{
		float2 size;
		auto& settings = ProjectSettings::GetProjectSettings().GalGame;
		size.x = static_cast<float>(settings.DesignWidth);
		size.y = static_cast<float>(settings.DesignHeight);

		return size;
	}

	float GameEngineCore::GetSpriteYOffset(float size_y)
	{
		return -( GetDesignSize().y - size_y) / 2.0f;
	}

	float GameEngineCore::GetSpriteXOffset(float size_x)
	{
		return -(GetDesignSize().x - size_x) / 2.0f;
	}

	IGalGameEngine* GameEngineCore::GetCurrentEngine()
	{
		return GameCoreImp::Get()->GameEngine;
	}

	void GameEngineCore::SetCurrentEngine(IGalGameEngine* engine)
	{
		GameCoreImp::Get()->GameEngine = engine;
	}
}
