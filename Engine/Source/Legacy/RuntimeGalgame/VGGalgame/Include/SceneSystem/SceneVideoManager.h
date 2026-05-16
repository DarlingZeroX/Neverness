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
#include "NNEngineLegacy/Include/Render/RenderCore.h"
#include "../Game.h" 
#include "VGGalgameCore/Include/GalGameContext.h"
#include <deque>

namespace VisionGal::GalGame
{
	class SceneVideoManager : public ISceneVideoManager
	{
	public:
		struct VideoLayer : ISceneVideoLayer
		{
			String name;
			std::vector<IGalVideo*> videos;

			void SetVolume(float volume) override;
			float GetVolume() override;
			void Clear() override;
			void Add(IGalVideo* audio) override;
			void StopPlay() override;
			bool Remove(IGalVideo* audio) override;
			bool IsPlayFinished() override;
		private:
			float m_Volume = 1.0f;
		};

		SceneVideoManager();
		~SceneVideoManager() override = default;

		void AddVideo(IGalVideo* video) override;
		bool RemoveVideo(IGalVideo* video) override;
		void ClearVideoLayer(const String& layer) override;
		void ClearAllVideo() override;

		void TraverseVideoLayer(const String& layer, const std::function<void(IGalVideo* audio)>& callback) override;
		void TraverseVideo(const std::function<void(IGalVideo* audio)>& callback) override;
		void AddVideoLayer(const String& layer) override;
		ISceneVideoLayer* GetVideoLayer(const String& layer) override;
	private:
		void Initialize();

		std::vector<VideoLayer> m_VideoLayers;
		std::unordered_map<String, int> m_VideoLayerIndexer;
	};

}
