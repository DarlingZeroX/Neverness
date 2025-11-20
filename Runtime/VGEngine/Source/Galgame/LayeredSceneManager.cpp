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

#include "Galgame/LayeredSceneManager.h"

namespace VisionGal::GalGame
{
	///////////////////////		SceneSpriteManager::SpriteLayer
	void SceneSpriteManager::SpriteLayer::Clear()
	{
		for (auto& ad : sprites)
		{
			if (ad != nullptr)
			{
				delete ad;
				ad = nullptr;
			}
		}

		sprites.clear();
	}

	bool SceneSpriteManager::SpriteLayer::Add(IGalGameSprite* sprite)
	{
		for (auto& sp : sprites)
		{
			if (sp == sprite)
			{
				return true;
				//delete sp;
				//sp = nullptr;
			}
		}

		sprites.push_back(sprite);
		return true;
	}

	bool SceneSpriteManager::SpriteLayer::Remove(IGalGameSprite* sprite)
	{
		for (auto& sp : sprites)
		{
			if (sp == sprite)
			{
				delete sp;
				sp = nullptr;
				return true;
			}
		}

		return false;
	}

	///////////////////////		SceneSpriteManager
	SceneSpriteManager::SceneSpriteManager()
	{
		Initialize();
	}

	void SceneSpriteManager::AddSprite(IGalGameSprite* sprite)
	{
		if (sprite == nullptr)
			return;

		// 如果图层不存在则创建
		std::string layer = sprite->GetResourceLayer();
		if (m_SpriteLayerIndexer.find(layer) == m_SpriteLayerIndexer.end())
		{
			AddSpriteLayer(layer);
		}

		auto& spriteLayer = m_SpriteLayers[m_SpriteLayerIndexer[layer]];

		spriteLayer.Add(sprite);
	}

	bool SceneSpriteManager::RemoveSprite(IGalGameSprite* sprite)
	{
		if (sprite == nullptr)
			return false;

		auto& spriteLayer = m_SpriteLayers[m_SpriteLayerIndexer[sprite->GetResourceLayer()]];

		return spriteLayer.Remove(sprite);
	}

	void SceneSpriteManager::Initialize()
	{
		AddSpriteLayer("Background");
		AddSpriteLayer("Foreground");
		AddSpriteLayer("SceneCharacterSpriteCurrent");
		AddSpriteLayer("SceneCharacterSpritePrev");
		AddSpriteLayer("Screen");
	}

	bool SceneSpriteManager::MoveSpriteToLayer(IGalGameSprite* sprite, const String& layer)
	{
		if (sprite == nullptr)
			return false;

		// 检查当前层和目标层是否存在
		if (m_SpriteLayerIndexer.find(sprite->GetResourceLayer()) == m_SpriteLayerIndexer.end() ||
			m_SpriteLayerIndexer.find(layer) == m_SpriteLayerIndexer.end())
		{
			return false;
		}

		auto& currentLayer = m_SpriteLayers[m_SpriteLayerIndexer[sprite->GetResourceLayer()]];
		auto& targetLayer = m_SpriteLayers[m_SpriteLayerIndexer[layer]];

		// 从当前层移除精灵
		auto it = std::find(currentLayer.sprites.begin(), currentLayer.sprites.end(), sprite);
		if (it != currentLayer.sprites.end())
		{
			currentLayer.sprites.erase(it);
		}

		// 添加到目标层
		targetLayer.sprites.push_back(sprite);
		sprite->SetResourceLayer(layer);
		return true;
	}

	void SceneSpriteManager::ClearSpriteLayer(const String& layer)
	{
		m_SpriteLayers[m_SpriteLayerIndexer[layer]].Clear();
	}

	void SceneSpriteManager::ClearAllSprite()
	{
		for (auto& layer : m_SpriteLayers)
			layer.Clear();
	}

	void SceneSpriteManager::TraverseSpriteLayer(const String& layer,
	                                             const std::function<void(IGalGameSprite* sprite)>& callback)
	{
		auto& spriteLayer = m_SpriteLayers[m_SpriteLayerIndexer[layer]];

		for (auto* sprite: spriteLayer.sprites)
		{
			if (sprite != nullptr)
			{
				callback(sprite);
			}
		}
	}

	void SceneSpriteManager::TraverseSprite(const std::function<void(IGalGameSprite* sprite)>& callback)
	{
		for (auto& layer : m_SpriteLayers)
		{
			for (auto* sprite : layer.sprites)
			{
				if (sprite != nullptr)
				{
					callback(sprite);
				}
			}
		}
	}

	void SceneSpriteManager::AddSpriteLayer(const String& layer)
	{
		m_SpriteLayerIndexer[layer] = m_SpriteLayers.size();
		m_SpriteLayers.emplace_back();
		m_SpriteLayers.back().name = layer;
	}

	ISceneSpriteLayer* SceneSpriteManager::GetSpriteLayer(const String& layer)
	{
		if (m_SpriteLayerIndexer.find(layer) != m_SpriteLayerIndexer.end())
		{
			return &m_SpriteLayers[m_SpriteLayerIndexer[layer]];
		}
	}


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
		Initialize();
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

	void SceneAudioManager::Initialize()
	{
		AddAudioLayer("BGM");
		AddAudioLayer("Voice");
		AddAudioLayer("Effect");
		AddAudioLayer("System");
	}

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

	///////////////////////		LayeredSceneManager
	LayeredSceneManager::LayeredSceneManager()
	{
		Initialize();
	}

	void LayeredSceneManager::ClearAll()
	{
		m_AudioManager.ClearAllAudio();
		m_SpriteManager.ClearAllSprite();
		ClearAllCharacter();
	}

	void LayeredSceneManager::ClearAllCharacter()
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

	void LayeredSceneManager::AddCharacter(IGalCharacter* character)
	{
		m_Characters.push_back(character);
	}

	void LayeredSceneManager::TraverseScene(std::function<void(IGalGameResource* resource)> callback)
	{
		m_SpriteManager.TraverseSprite(callback);
		m_AudioManager.TraverseAudio(callback);
	}

	void LayeredSceneManager::TraverseCharacter(const std::function<void(IGalCharacter* character)>& callback)
	{
		for (auto& character: m_Characters)
		{
			callback(character);
		}
	}

	ISceneAudioLayer* LayeredSceneManager::GetAudioLayer(const String& layer)
	{
		return m_AudioManager.GetAudioLayer(layer);
	}

	ISceneSpriteLayer* LayeredSceneManager::GetSpriteLayer(const String& layer)
	{
		return m_SpriteManager.GetSpriteLayer(layer);
	}

	void LayeredSceneManager::Initialize()
	{
		//AddSpriteLayer("Background");
		//AddSpriteLayer("Foreground");
		//AddSpriteLayer("SceneCharacterSpriteCurrent");
		//AddSpriteLayer("SceneCharacterSpritePrev");
		//AddSpriteLayer("Screen");
		//
		//AddAudioLayer("BGM");
		//AddAudioLayer("Voice");
		//AddAudioLayer("Effect");
		//AddAudioLayer("System");
	}
}
