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
#include "VGGalgameCore/Interface/IGameSystem.h"
#include "../Game.h" 
#include "VGGalgameCore/Include/GalGameContext.h"

namespace VisionGal::GalGame
{
	struct SceneAudioManager : public ISceneAudioManager
	{
		struct AudioLayer : public ISceneAudioLayer
		{
			String name;
			std::vector<IGalAudio*> audios;

			void SetVolume(float volume) override;
			float GetVolume() override;
			void Clear() override;
			void Add(IGalAudio* audio) override;
			void StopPlay() override;
			bool Remove(IGalAudio* audio) override;
			bool IsPlayFinished() override;
		private:
			float m_Volume = 1.0f;
		};

		SceneAudioManager();
		~SceneAudioManager() override = default;

		void AddAudio(IGalAudio* audio) override;
		bool RemoveAudio(IGalAudio* audio) override;
		void ClearSoundLayer(const String& layer) override;
		void ClearAllAudio() override;

		void TraverseAudioLayer(const String& layer, const std::function<void(IGalAudio* audio)>& callback) override;
		void TraverseAudio(const std::function<void(IGalAudio* audio)>& callback) override;
		void AddAudioLayer(const String& layer) override;
		ISceneAudioLayer* GetAudioLayer(const String& layer) override;

		void OnUpdate();
		void Initialize(const Ref<GalGameContext>& ctx);
	private:
		Ref<GalGameContext> m_GalGameContext = nullptr;

		std::vector<AudioLayer> m_AudioLayers;
		std::unordered_map<String, int> m_AudioLayerIndexer;
	};

}
