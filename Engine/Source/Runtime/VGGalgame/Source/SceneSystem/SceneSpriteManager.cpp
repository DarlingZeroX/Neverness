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

#include "SceneSystem/SceneSpriteManager.h"

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

		for (auto* sprite : spriteLayer.sprites)
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
}