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

#include "SceneSystem/SceneAudioManager.h"

namespace VisionGal::GalGame
{
	///////////////////////		SceneAudioManager::AudioLayer
	void SceneAudioManager::AudioLayer::SetVolume(float volume)
	{
		m_Volume = volume;

		for (auto& audio : audios)
		{
			auto* galAudio = dynamic_cast<GalAudio*>(audio);
			if (galAudio != nullptr)
			{
				galAudio->SetVolume(m_Volume);
			}
		}
	}

	float SceneAudioManager::AudioLayer::GetVolume()
	{
		return m_Volume;
	}

	void SceneAudioManager::AudioLayer::Clear()
	{
		for (auto& ad : audios)
		{
			if (ad != nullptr)
			{
				delete ad;
				ad = nullptr;
			}
		}

		audios.clear();
	}

	void SceneAudioManager::AudioLayer::Add(IGalGameAudio* audio)
	{
		H_ASSERT_NOT_NULL(audio);

		auto* galAudio = dynamic_cast<GalAudio*>(audio);
		galAudio->SetVolume(m_Volume);

		audios.push_back(audio);
	}

	void SceneAudioManager::AudioLayer::StopPlay()
	{
		for (auto& audio : audios)
		{
			auto* galAudio = dynamic_cast<GalAudio*>(audio);
			if (galAudio != nullptr)
			{
				galAudio->Stop();
			}
		}
	}

	bool SceneAudioManager::AudioLayer::Remove(IGalGameAudio* audio)
	{
		for (auto& ad : audios)
		{
			if (ad == audio)
			{
				delete ad;
				ad = nullptr;
				return true;
			}
		}

		return false;
	}

	bool SceneAudioManager::AudioLayer::IsPlayFinished()
	{
		for (auto& audio : audios)
		{
			auto* galAudio = dynamic_cast<GalAudio*>(audio);
			if (galAudio != nullptr && galAudio->IsPlayingAudio())
			{
				return false;
			}
		}

		return true;
	}

	///////////////////////		SceneAudioManager
	SceneAudioManager::SceneAudioManager()
	{
		AddAudioLayer("BGM");
		AddAudioLayer("Voice");
		AddAudioLayer("Effect");
		AddAudioLayer("System");
	}

	void SceneAudioManager::AddAudio(IGalGameAudio* audio)
	{
		if (audio == nullptr)
			return;

		// 如果图层不存在则创建
		std::string layer = audio->GetResourceLayer();
		if (m_AudioLayerIndexer.find(layer) == m_AudioLayerIndexer.end())
		{
			AddAudioLayer(layer);
		}

		auto& audioLayer = m_AudioLayers[m_AudioLayerIndexer[audio->GetResourceLayer()]];

		// 如果是语音图层，则停止当前所有语音播放
		if (audio->GetResourceLayer() == "Voice")
		{
			audioLayer.StopPlay();
		}

		audioLayer.Add(audio);
	}

	bool SceneAudioManager::RemoveAudio(IGalGameAudio* audio)
	{
		if (audio == nullptr)
			return false;

		auto& audioLayer = m_AudioLayers[m_AudioLayerIndexer[audio->GetResourceLayer()]];

		return audioLayer.Remove(audio);
	}

	void SceneAudioManager::ClearSoundLayer(const String& layer)
	{
		m_AudioLayers[m_AudioLayerIndexer[layer]].Clear();
	}

	void SceneAudioManager::ClearAllAudio()
	{
		for (auto& audio : m_AudioLayers)
			audio.Clear();
	}

	void SceneAudioManager::TraverseAudioLayer(const String& layer,
		const std::function<void(IGalGameAudio* audio)>& callback)
	{
		auto& audioLayer = m_AudioLayers[m_AudioLayerIndexer[layer]];

		for (auto* audio : audioLayer.audios)
		{
			if (audio != nullptr)
			{
				callback(audio);
			}
		}
	}

	void SceneAudioManager::TraverseAudio(const std::function<void(IGalGameAudio* audio)>& callback)
	{
		for (auto& layer : m_AudioLayers)
		{
			for (auto* sound : layer.audios)
			{
				if (sound != nullptr)
				{
					callback(sound);
				}
			}
		}
	}

	void SceneAudioManager::AddAudioLayer(const String& layer)
	{
		m_AudioLayerIndexer[layer] = m_AudioLayers.size();
		m_AudioLayers.emplace_back();
		m_AudioLayers.back().name = layer;
	}

	ISceneAudioLayer* SceneAudioManager::GetAudioLayer(const String& layer)
	{
		if (m_AudioLayerIndexer.find(layer) != m_AudioLayerIndexer.end())
		{
			return &m_AudioLayers[m_AudioLayerIndexer[layer]];
		}

		return nullptr;
	}

	void SceneAudioManager::OnUpdate()
	{
		H_ASSERT_NOT_NULL(m_GalGameContext);

		m_GalGameContext->runtimeState.IsVoicing = !GetAudioLayer("Voice")->IsPlayFinished();
	}

	void SceneAudioManager::Initialize(const Ref<GalGameContext>& ctx)
	{
		m_GalGameContext = ctx;
	}

}
