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

#include "SceneSystem/LayeredSceneSystem.h"
#include <NNKernel/Interface/HAssert.h>

namespace VisionGal::GalGame
{
	///////////////////////		LayeredSceneManager
	LayeredSceneSystem::LayeredSceneSystem()
	{
		//Initialize();
	}

	void LayeredSceneSystem::ClearAll()
	{
		m_AudioManager.ClearAllAudio();
		m_SpriteManager.ClearAllSprite();
		m_VideoManager.ClearAllVideo();
		ClearAllCharacter();
	}

	void LayeredSceneSystem::ClearAllCharacter()
	{
		for (auto& character: m_Characters)
		{
			if (character != nullptr)
			{
				delete character;
				character = nullptr;
			}
		}

		m_Characters.clear();
	}

	void LayeredSceneSystem::AddCharacter(IGalCharacter* character)
	{
		m_Characters.push_back(character);
	}

	void LayeredSceneSystem::TraverseScene(std::function<void(IGalGameResource* resource)> callback)
	{
		m_SpriteManager.TraverseSprite(callback);
		m_AudioManager.TraverseAudio(callback);
	}

	void LayeredSceneSystem::TraverseCharacter(const std::function<void(IGalCharacter* character)>& callback)
	{
		for (auto& character: m_Characters)
		{
			callback(character);
		}
	}

	void LayeredSceneSystem::OnUpdate()
	{
		m_AudioManager.OnUpdate();
	}

	ISceneAudioLayer* LayeredSceneSystem::GetAudioLayer(const String& layer)
	{
		return m_AudioManager.GetAudioLayer(layer);
	}

	ISceneSpriteLayer* LayeredSceneSystem::GetSpriteLayer(const String& layer)
	{
		return m_SpriteManager.GetSpriteLayer(layer);
	}

	void LayeredSceneSystem::Initialize(const Ref<GalGameContext>& ctx)
	{
		H_ASSERT_NOT_NULL(ctx.get());
		m_GalGameContext = ctx;
		m_AudioManager.Initialize(ctx);

		m_GalGameContext->engineEventBus.OnStoryScriptEvent.Subscribe([this](const GalGameScriptEvent& evt)
			{
				switch (evt.EventType)
				{
				case GalGameScriptEventType::OnScriptStartLoad:
					ClearAllCharacter();
					ClearAll();
					break;
				}
			});
	}
}
