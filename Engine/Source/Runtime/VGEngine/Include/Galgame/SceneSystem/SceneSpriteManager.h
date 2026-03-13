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
	class SceneSpriteManager : public ISceneSpriteManager
	{
	public:
		struct SpriteLayer : public ISceneSpriteLayer
		{
			String name;
			std::vector<IGalGameSprite*> sprites;

			void Clear() override;
			bool Add(IGalGameSprite* sprite) override;
			bool Remove(IGalGameSprite* sprite) override;
		};

		SceneSpriteManager();
		~SceneSpriteManager() override = default;

		void AddSprite(IGalGameSprite* sprite) override;
		bool RemoveSprite(IGalGameSprite* sprite) override;
		bool MoveSpriteToLayer(IGalGameSprite* sprite, const String& layer) override;
		void ClearSpriteLayer(const String& layer) override;
		void ClearAllSprite() override;

		void TraverseSpriteLayer(const String& layer, const std::function<void(IGalGameSprite* sprite)>& callback) override;
		void TraverseSprite(const std::function<void(IGalGameSprite* sprite)>& callback) override;
		void AddSpriteLayer(const String& layer) override;
		ISceneSpriteLayer* GetSpriteLayer(const String& layer) override;
	private:
		void Initialize();
	private:
		std::deque<SpriteLayer>  m_SpriteLayers;
		std::unordered_map<String, int> m_SpriteLayerIndexer;
	};

}
