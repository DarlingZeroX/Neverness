#include "Galgame/LayeredSceneManager.h"

namespace VisionGal::GalGame
{
	LayeredSceneManager::LayeredSceneManager()
	{
		Initialize();
	}

	void LayeredSceneManager::ShowSprite(const String& layer, GameActor* actor)
	{
		//auto& sprites = m_Sprite[m_SpriteLayer[layer]];
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
		if (m_SpriteLayer.find(layer) == m_SpriteLayer.end())
		{
			AddSpriteLayer(layer);
		}

		auto& sprites = m_Sprite[m_SpriteLayer[layer]];

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

	void LayeredSceneManager::AddAudio(IGalGameAudio* audio)
	{
		if (audio == nullptr)
			return;

		// 如果图层不存在则创建
		std::string layer = audio->GetResourceLayer();
		if (m_AudioLayer.find(layer) == m_AudioLayer.end())
		{
			AddAudioLayer(layer);
		}

		auto& audioLayer = m_Audio[m_AudioLayer[audio->GetResourceLayer()]];

		// 如果是语音图层，则停止当前所有语音播放
		if (audio->GetResourceLayer() == "Voice")
		{
			audioLayer.StopPlay();
		}

		audioLayer.AddAudio(audio);
	}

	bool LayeredSceneManager::RemoveSprite(IGalGameSprite* sprite)
	{
		if (sprite == nullptr)
			return false;

		auto& sprites = m_Sprite[m_SpriteLayer[sprite->GetResourceLayer()]];
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

	bool LayeredSceneManager::RemoveAudio(IGalGameAudio* audio)
	{
		if (audio == nullptr)
			return false;

		auto& audioLayer = m_Audio[m_AudioLayer[audio->GetResourceLayer()]];
		audioLayer.RemoveAudio(audio);

		return false;
	}

	bool LayeredSceneManager::MoveSpriteToLayer(IGalGameSprite* sprite, const String& layer)
	{
		if (sprite == nullptr)
			return false;

        // 检查当前层和目标层是否存在
        if (m_SpriteLayer.find(sprite->GetResourceLayer()) == m_SpriteLayer.end() ||
        m_SpriteLayer.find(layer) == m_SpriteLayer.end())
        {
			return false;
        }

        auto& currentLayer = m_Sprite[m_SpriteLayer[sprite->GetResourceLayer()]];
        auto& targetLayer = m_Sprite[m_SpriteLayer[layer]];

		// 从当前层移除精灵
		auto it = std::find(currentLayer.begin(), currentLayer.end(), sprite);
		if (it != currentLayer.end())
		{
			currentLayer.erase(it);
		}

		// 添加到目标层
		targetLayer.push_back(sprite);
		sprite->SetResourceLayer(layer);
		return true;
	}

	void LayeredSceneManager::ClearSpriteLayer(const String& layer)
	{
		m_Sprite[m_SpriteLayer[layer]].clear();
	}

	void LayeredSceneManager::ClearSoundLayer(const String& layer)
	{
		m_Audio[m_AudioLayer[layer]].Clear();
	}

	void LayeredSceneManager::ClearAllSprite()
	{
		TraverseSprite([this](IGalGameSprite* sprite)
			{
				delete sprite;
			});

		for (auto& layer : m_Sprite)
			layer.clear();
	}

	void LayeredSceneManager::ClearAllAudio()
	{
		TraverseAudio([this](IGalGameAudio* audio)
			{
				delete audio;
			});

		for (auto& audio : m_Audio)
			audio.Clear();
	}

	void LayeredSceneManager::ClearAll()
	{
		ClearAllAudio();
		ClearAllSprite();
	}

	void LayeredSceneManager::TraverseSpriteLayer(const String& layer, const std::function<void(IGalGameSprite* galSprite)>& callback)
	{
		auto& sprites = m_Sprite[m_SpriteLayer[layer]];

		for (auto* sprite: sprites)
		{
			if (sprite != nullptr)
			{
				callback(sprite);
			}
		}
	}

	void LayeredSceneManager::TraverseSprite(const std::function<void(IGalGameSprite* galSprite)>& callback)
	{
		for (auto& layer : m_Sprite)
		{
			for (auto* sprite : layer)
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
		auto& audioLayer = m_Audio[m_AudioLayer[layer]];

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
		for (auto& layer : m_Audio)
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

	void LayeredSceneManager::AddSpriteLayer(const String& layer)
	{
		m_SpriteLayer[layer] = m_Sprite.size();
		m_Sprite.emplace_back();
	}

	void LayeredSceneManager::AddAudioLayer(const String& layer)
	{
		m_AudioLayer[layer] = m_Audio.size();
		m_Audio.emplace_back();
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

	void LayeredSceneManager::AudioLayer::Clear()
	{
		audios.clear();
	}

	void LayeredSceneManager::AudioLayer::AddAudio(IGalGameAudio* audio)
	{
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
	}
}
