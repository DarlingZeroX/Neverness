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
#include "../Interface/GalgameInterface.h"
#include "../../Render/RenderCore.h"
#include "../Game.h" 
#include "../Core/GalGameContext.h"
#include <deque>

namespace VisionGal::GalGame
{
	class SceneVideoManager : public ISceneVideoManager
	{
	public:
		struct VideoLayer : ISceneVideoLayer
		{
			String name;
			std::vector<IGalGameVideo*> videos;

			void SetVolume(float volume) override;
			float GetVolume() override;
			void Clear() override;
			void Add(IGalGameVideo* audio) override;
			void StopPlay() override;
			bool Remove(IGalGameVideo* audio) override;
			bool IsPlayFinished() override;
		private:
			float m_Volume = 1.0f;
		};

		SceneVideoManager();
		~SceneVideoManager() override = default;

		void AddVideo(IGalGameVideo* video) override;
		bool RemoveVideo(IGalGameVideo* video) override;
		void ClearVideoLayer(const String& layer) override;
		void ClearAllVideo() override;

		void TraverseVideoLayer(const String& layer, const std::function<void(IGalGameVideo* audio)>& callback) override;
		void TraverseVideo(const std::function<void(IGalGameVideo* audio)>& callback) override;
		void AddVideoLayer(const String& layer) override;
		ISceneVideoLayer* GetVideoLayer(const String& layer) override;
	private:
		void Initialize();

		std::vector<VideoLayer> m_VideoLayers;
		std::unordered_map<String, int> m_VideoLayerIndexer;
	};

}
