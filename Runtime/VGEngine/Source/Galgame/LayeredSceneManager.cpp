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
	LayeredSceneManager::LayeredSceneManager()
	{
		Initialize();
	}

	void LayeredSceneManager::ShowSprite(const String& layer, GameActor* actor)
	{
		//auto& sprites = m_Sprite[m_SpriteLayerIndexer[layer]];
		//
		//for (auto& sprite : sprites)
		//{
		//	if (sprite == actor)
		//	{
		//		sprite = nullptr;
		//	}
		//}
		//
		//sprites.push_back(actor);
	}

	void LayeredSceneManager::AddSprite(IGalGameSprite* sprite)
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

		spriteLayer.AddSprite(sprite);
	}

	void LayeredSceneManager::AddAudio(IGalGameAudio* audio)
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

		audioLayer.AddAudio(audio);
	}

	void LayeredSceneManager::AddCharacter(IGalCharacter* character)
	{
		m_Characters.push_back(character);
	}

	bool LayeredSceneManager::RemoveSprite(IGalGameSprite* sprite)
	{
		if (sprite == nullptr)
			return false;

		auto& spriteLayer = m_SpriteLayers[m_SpriteLayerIndexer[sprite->GetResourceLayer()]];

		return spriteLayer.RemoveSprite(sprite);
	}

	bool LayeredSceneManager::RemoveAudio(IGalGameAudio* audio)
	{
		if (audio == nullptr)
			return false;

		auto& audioLayer = m_AudioLayers[m_AudioLayerIndexer[audio->GetResourceLayer()]];

		return audioLayer.RemoveAudio(audio);
	}

	bool LayeredSceneManager::MoveSpriteToLayer(IGalGameSprite* sprite, const String& layer)
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

	void LayeredSceneManager::ClearSpriteLayer(const String& layer)
	{
		m_SpriteLayers[m_SpriteLayerIndexer[layer]].Clear();
	}

	void LayeredSceneManager::ClearSoundLayer(const String& layer)
	{
		m_AudioLayers[m_AudioLayerIndexer[layer]].Clear();
	}

	void LayeredSceneManager::ClearAllSprite()
	{
		for (auto& layer : m_SpriteLayers)
			layer.Clear();
	}

	void LayeredSceneManager::ClearAllAudio()
	{
		for (auto& audio : m_AudioLayers)
			audio.Clear();
	}

	void LayeredSceneManager::ClearAll()
	{
		ClearAllAudio();
		ClearAllSprite();
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

	void LayeredSceneManager::TraverseSpriteLayer(const String& layer, const std::function<void(IGalGameSprite* galSprite)>& callback)
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

	void LayeredSceneManager::TraverseSprite(const std::function<void(IGalGameSprite* galSprite)>& callback)
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

	void LayeredSceneManager::TraverseAudioLayer(const String& layer, const std::function<void(IGalGameAudio* audio)>& callback)
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

	void LayeredSceneManager::TraverseAudio(const std::function<void(IGalGameAudio* audio)>& callback)
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

	void LayeredSceneManager::TraverseScene(std::function<void(IGalGameResource* resource)> callback)
	{
		TraverseSprite(callback);
		TraverseAudio(callback);
	}

	void LayeredSceneManager::TraverseCharacter(const std::function<void(IGalCharacter* character)>& callback)
	{
		for (auto& character: m_Characters)
		{
			callback(character);
		}
	}

	void LayeredSceneManager::AddSpriteLayer(const String& layer)
	{
		m_SpriteLayerIndexer[layer] = m_SpriteLayers.size();
		m_SpriteLayers.emplace_back();
		m_SpriteLayers.back().name = layer;
	}

	void LayeredSceneManager::AddAudioLayer(const String& layer)
	{
		m_AudioLayerIndexer[layer] = m_AudioLayers.size();
		m_AudioLayers.emplace_back();
		m_AudioLayers.back().name = layer;
	}

	LayeredSceneManager::AudioLayer* LayeredSceneManager::GetAudioLayer(const String& layer)
	{
		if (m_AudioLayerIndexer.find(layer) != m_AudioLayerIndexer.end())
		{
			return &m_AudioLayers[m_AudioLayerIndexer[layer]];
		}
	}

	LayeredSceneManager::SpriteLayer* LayeredSceneManager::GetSpriteLayer(const String& layer)
	{
		if (m_SpriteLayerIndexer.find(layer) != m_SpriteLayerIndexer.end())
		{
			return &m_SpriteLayers[m_SpriteLayerIndexer[layer]];
		}
	}

	void LayeredSceneManager::Initialize()
	{
		AddSpriteLayer("Background");
		AddSpriteLayer("Foreground");
		AddSpriteLayer("SceneCharacterSpriteCurrent");
		AddSpriteLayer("SceneCharacterSpritePrev");
		AddSpriteLayer("Screen");

		AddAudioLayer("BGM");
		AddAudioLayer("Voice");
		AddAudioLayer("Effect");
		AddAudioLayer("System");
	}

	void LayeredSceneManager::SpriteLayer::Clear()
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

	void LayeredSceneManager::SpriteLayer::AddSprite(IGalGameSprite* sprite)
	{
		for (auto& sp : sprites)
		{
			if (sp == sprite)
			{
				delete sp;
				sp = nullptr;
			}
		}

		sprites.push_back(sprite);
	}

	bool LayeredSceneManager::SpriteLayer::RemoveSprite(IGalGameSprite* sprite)
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

	void LayeredSceneManager::AudioLayer::SetVolume(float volume)
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

	float LayeredSceneManager::AudioLayer::GetVolume()
	{
		return m_Volume;
	}

	void LayeredSceneManager::AudioLayer::Clear()
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

	void LayeredSceneManager::AudioLayer::AddAudio(IGalGameAudio* audio)
	{
		H_ASSERT_NOT_NULL(audio);

		auto* galAudio = dynamic_cast<GalAudio*>(audio);
		galAudio->SetVolume(m_Volume);

		audios.push_back(audio);
	}

	void LayeredSceneManager::AudioLayer::StopPlay()
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

	bool LayeredSceneManager::AudioLayer::RemoveAudio(IGalGameAudio* audio)
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
}
