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
#include "Interface.h"
#include "../EngineConfig.h"
#include "../Scene/Scene.h"
#include "../Render/RenderCore.h"
#include <deque>
#include "Game.h" 

namespace VisionGal::GalGame
{
	class SceneSpriteManager: public ISceneSpriteManager
	{
	public:
		struct SpriteLayer
		{
			String name;
			std::vector<IGalGameSprite*> sprites;

			void Clear();
			void AddSprite(IGalGameSprite* sprite);
			bool RemoveSprite(IGalGameSprite* sprite);
		};

		SceneSpriteManager();
		~SceneSpriteManager() override = default;

		void AddSprite(IGalGameSprite* sprite) override;
		bool RemoveSprite(IGalGameSprite* sprite) override;
		bool MoveSpriteToLayer(IGalGameSprite* sprite, const String& layer) override;
		void ClearSpriteLayer(const String& layer) override;
		void ClearAllSprite() override;

		void TraverseSpriteLayer(const String& layer, const std::function<void(IGalGameSprite* sprite)>& callback) override;
		void TraverseSprite(const std::function<void(IGalGameSprite* sprite)>& callback) override;
		void AddSpriteLayer(const String& layer);
		SpriteLayer* GetSpriteLayer(const String& layer);
	private:
		void Initialize();
	private:
		std::deque<SpriteLayer>  m_SpriteLayers;
		std::unordered_map<String, int> m_SpriteLayerIndexer;
	};

	struct SceneAudioManager: public ISceneAudioManager
	{
		struct AudioLayer
		{
			String name;
			std::vector<IGalGameAudio*> audios;

			void SetVolume(float volume);
			float GetVolume();
			void Clear();
			void AddAudio(IGalGameAudio* audio);
			void StopPlay();
			bool RemoveAudio(IGalGameAudio* audio);
			bool IsPlayFinished();
		private:
			float m_Volume = 1.0f;
		};

		SceneAudioManager();
		~SceneAudioManager() override = default;

		void AddAudio(IGalGameAudio* audio) override;
		bool RemoveAudio(IGalGameAudio* audio) override;
		void ClearSoundLayer(const String& layer) override;
		void ClearAllAudio() override;

		void TraverseAudioLayer(const String& layer, const std::function<void(IGalGameAudio* audio)>& callback) override;
		void TraverseAudio(const std::function<void(IGalGameAudio* audio)>& callback) override;
		void AddAudioLayer(const String& layer);
		AudioLayer* GetAudioLayer(const String& layer);
	private:
		void Initialize();

		std::vector<AudioLayer> m_AudioLayers;
		std::unordered_map<String, int> m_AudioLayerIndexer;
	};

	class VG_ENGINE_API LayeredSceneManager: public ILayeredSceneManager
	{
	public:
		LayeredSceneManager();
		~LayeredSceneManager() override = default;
		LayeredSceneManager(const LayeredSceneManager&) = delete;
		LayeredSceneManager& operator=(const LayeredSceneManager&) = delete;
		LayeredSceneManager(LayeredSceneManager&&) noexcept = default;
		LayeredSceneManager& operator=(LayeredSceneManager&&) noexcept = default;

		void AddCharacter(IGalCharacter* character);
		void ClearAll() override;
		void ClearAllCharacter();
		void TraverseScene(std::function<void(IGalGameResource* actor)> callback) override;
		void TraverseCharacter(const std::function<void(IGalCharacter* character)>& callback);

		//void AddSpriteLayer(const String& layer);
		//void AddAudioLayer(const String& layer);

		SceneAudioManager::AudioLayer* GetAudioLayer(const String& layer);
		SceneSpriteManager::SpriteLayer* GetSpriteLayer(const String& layer);

		ISceneSpriteManager* GetSpriteManager() override { return &m_SpriteManager; }
		ISceneAudioManager* GetAudioManager() override { return &m_AudioManager; }
	public:
		//void Initialize(Scene* scene);
		//void OnRender();
		//void OnUpdate();
		void Initialize();
	private:
		SceneSpriteManager m_SpriteManager;
		SceneAudioManager m_AudioManager;

		std::vector<IGalCharacter*> m_Characters;			// 当前游戏中的所有角色列表，存储了所有创建的 GalCharacter 实例。
	};


}
