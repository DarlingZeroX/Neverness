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
#include "SceneAudioManager.h"
#include "SceneSpriteManager.h"
#include "SceneVideoManager.h"

namespace VisionGal::GalGame
{
	class VG_GALGAME_API LayeredSceneSystem: public ILayeredSceneManager
	{
	public:
		LayeredSceneSystem();
		~LayeredSceneSystem() override = default;
		LayeredSceneSystem(const LayeredSceneSystem&) = delete;
		LayeredSceneSystem& operator=(const LayeredSceneSystem&) = delete;
		LayeredSceneSystem(LayeredSceneSystem&&) noexcept = default;
		LayeredSceneSystem& operator=(LayeredSceneSystem&&) noexcept = default;

		void AddCharacter(IGalCharacter* character);
		void ClearAll() override;
		void ClearAllCharacter();
		void TraverseScene(std::function<void(IGalGameResource* actor)> callback) override;
		void TraverseCharacter(const std::function<void(IGalCharacter* character)>& callback);
		void OnUpdate();
		
		ISceneAudioLayer* GetAudioLayer(const String& layer);
		ISceneSpriteLayer* GetSpriteLayer(const String& layer);

		ISceneSpriteManager* GetSpriteManager() override { return &m_SpriteManager; }
		ISceneAudioManager* GetAudioManager() override { return &m_AudioManager; }
		ISceneVideoManager* GetVideoManager() override { return &m_VideoManager; }
	public:
		void Initialize(const Ref<GalGameContext>& ctx);
	private:
		Ref<GalGameContext> m_GalGameContext;

		SceneSpriteManager m_SpriteManager;
		SceneAudioManager m_AudioManager;
		SceneVideoManager m_VideoManager;

		std::vector<IGalCharacter*> m_Characters;			// 当前游戏中的所有角色列表，存储了所有创建的 GalCharacter 实例。
	};

}
