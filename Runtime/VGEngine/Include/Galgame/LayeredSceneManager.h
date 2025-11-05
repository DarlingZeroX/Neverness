#pragma once
#include "Interface.h"
#include "../EngineConfig.h"
#include "../Scene/Scene.h"
#include "../Render/RenderCore.h"
#include <deque>
#include "Game.h"

namespace VisionGal::GalGame
{
	class VG_ENGINE_API LayeredSceneManager: public ILayeredSceneManager
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

		private:
			float m_Volume = 1.0f;
		};
	public:
		LayeredSceneManager();
		~LayeredSceneManager() override = default;
		LayeredSceneManager(const LayeredSceneManager&) = delete;
		LayeredSceneManager& operator=(const LayeredSceneManager&) = delete;
		LayeredSceneManager(LayeredSceneManager&&) noexcept = default;
		LayeredSceneManager& operator=(LayeredSceneManager&&) noexcept = default;

		void ShowSprite(const String& layer, GameActor* actor) override;
		void AddSprite(IGalGameSprite* sprite) override;
		void AddAudio(IGalGameAudio* audio) override;

		bool RemoveSprite(IGalGameSprite* sprite) override;
		bool RemoveAudio(IGalGameAudio* audio) override;

		bool MoveSpriteToLayer(IGalGameSprite* sprite, const String& layer) override;

		void ClearSpriteLayer(const String& layer) override;
		void ClearSoundLayer(const String& layer) override;
		void ClearAllSprite() override;
		void ClearAllAudio() override;
		void ClearAll() override;

		void TraverseSpriteLayer(const String& layer, const std::function<void(IGalGameSprite* sprite)>& callback) override;
		void TraverseSprite(const std::function<void(IGalGameSprite* sprite)>& callback) override;
		void TraverseAudioLayer(const String& layer, const std::function<void(IGalGameAudio* audio)>& callback) override;
		void TraverseAudio(const std::function<void(IGalGameAudio* audio)>& callback) override;
		void TraverseScene(std::function<void(IGalGameResource* actor)> callback) override;

		void AddSpriteLayer(const String& layer);
		void AddAudioLayer(const String& layer);

		AudioLayer* GetAudioLayer(const String& layer);
		SpriteLayer* GetSpriteLayer(const String& layer);
	public:
		//void Initialize(Scene* scene);
		//void OnRender();
		//void OnUpdate();
		void Initialize();
	private:

		std::deque<SpriteLayer>  m_SpriteLayers;
		std::vector<AudioLayer> m_AudioLayers;

		std::unordered_map<String, int> m_SpriteLayerIndexer;
		std::unordered_map<String, int> m_AudioLayerIndexer;
	};


}
