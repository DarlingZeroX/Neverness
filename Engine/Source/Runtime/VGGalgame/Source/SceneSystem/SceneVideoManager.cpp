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

#include "SceneSystem/SceneVideoManager.h"

namespace VisionGal::GalGame
{
	///////////////////////		SceneVideoManager::VideoLayer
	void SceneVideoManager::VideoLayer::SetVolume(float volume)
	{
		m_Volume = volume;

		for (auto& video : videos)
		{
			auto* galVideo = dynamic_cast<GalVideo*>(video);
			if (galVideo != nullptr)
			{
				galVideo->SetVolume(m_Volume);
			}
		}
	}

	float SceneVideoManager::VideoLayer::GetVolume()
	{
		return m_Volume;
	}

	void SceneVideoManager::VideoLayer::Clear()
	{
		for (auto& ad : videos)
		{
			if (ad != nullptr)
			{
				delete ad;
				ad = nullptr;
			}
		}

		videos.clear();
	}

	void SceneVideoManager::VideoLayer::Add(IGalGameVideo* audio)
	{
		H_ASSERT_NOT_NULL(audio);

		auto* galVideo = dynamic_cast<GalVideo*>(audio);
		galVideo->SetVolume(m_Volume);

		videos.push_back(audio);
	}

	void SceneVideoManager::VideoLayer::StopPlay()
	{
		for (auto& video : videos)
		{
			auto* galVideo = dynamic_cast<GalVideo*>(video);
			if (galVideo != nullptr)
			{
				galVideo->Stop();
			}
		}
	}

	bool SceneVideoManager::VideoLayer::Remove(IGalGameVideo* video)
	{
		for (auto& vd : videos)
		{
			if (vd == video)
			{
				delete vd;
				vd = nullptr;
				return true;
			}
		}

		return false;
	}

	bool SceneVideoManager::VideoLayer::IsPlayFinished()
	{
		for (auto& video : videos)
		{
			auto* galVideo = dynamic_cast<GalVideo*>(video);
			if (galVideo != nullptr && galVideo->IsPlaying() == false)
			{
				return false;
			}
		}

		return true;
	}

	///////////////////////		SceneVideoManager
	SceneVideoManager::SceneVideoManager()
	{
		Initialize();
	}

	void SceneVideoManager::AddVideo(IGalGameVideo* video)
	{
		if (video == nullptr)
			return;

		// 如果图层不存在则创建
		std::string layer = video->GetResourceLayer();
		if (m_VideoLayerIndexer.find(layer) == m_VideoLayerIndexer.end())
		{
			AddVideoLayer(layer);
		}

		auto& videoLayer = m_VideoLayers[m_VideoLayerIndexer[layer]];

		videoLayer.Add(video);
	}

	bool SceneVideoManager::RemoveVideo(IGalGameVideo* video)
	{
		if (video == nullptr)
			return false;

		auto& videoLayer = m_VideoLayers[m_VideoLayerIndexer[video->GetResourceLayer()]];

		return videoLayer.Remove(video);
	}

	void SceneVideoManager::ClearVideoLayer(const String& layer)
	{
		m_VideoLayers[m_VideoLayerIndexer[layer]].Clear();
	}

	void SceneVideoManager::ClearAllVideo()
	{
		for (auto& audio : m_VideoLayers)
			audio.Clear();
	}

	void SceneVideoManager::TraverseVideoLayer(const String& layer,
		const std::function<void(IGalGameVideo* audio)>& callback)
	{
		auto& videoLayer = m_VideoLayers[m_VideoLayerIndexer[layer]];

		for (auto* video : videoLayer.videos)
		{
			if (video != nullptr)
			{
				callback(video);
			}
		}
	}

	void SceneVideoManager::TraverseVideo(const std::function<void(IGalGameVideo* audio)>& callback)
	{
		for (auto& layer : m_VideoLayers)
		{
			for (auto* sound : layer.videos)
			{
				if (sound != nullptr)
				{
					callback(sound);
				}
			}
		}
	}

	void SceneVideoManager::AddVideoLayer(const String& layer)
	{
		m_VideoLayerIndexer[layer] = m_VideoLayers.size();
		m_VideoLayers.emplace_back();
		m_VideoLayers.back().name = layer;
	}

	ISceneVideoLayer* SceneVideoManager::GetVideoLayer(const String& layer)
	{
		if (m_VideoLayerIndexer.find(layer) != m_VideoLayerIndexer.end())
		{
			return &m_VideoLayers[m_VideoLayerIndexer[layer]];
		}

		return nullptr;
	}

	void SceneVideoManager::Initialize()
	{
		AddVideoLayer("Background");
		AddVideoLayer("Foreground");
		AddVideoLayer("Screen");
	}

}
