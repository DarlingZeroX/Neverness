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
	class SceneSpriteManager : public ISceneSpriteManager
	{
	public:
		struct SpriteLayer : public ISceneSpriteLayer
		{
			String name;
			std::vector<IGalSprite*> sprites;

			void Clear() override;
			bool Add(IGalSprite* sprite) override;
			bool Remove(IGalSprite* sprite) override;
		};

		SceneSpriteManager();
		~SceneSpriteManager() override = default;

		void AddSprite(IGalSprite* sprite) override;
		bool RemoveSprite(IGalSprite* sprite) override;
		bool MoveSpriteToLayer(IGalSprite* sprite, const String& layer) override;
		void ClearSpriteLayer(const String& layer) override;
		void ClearAllSprite() override;

		void TraverseSpriteLayer(const String& layer, const std::function<void(IGalSprite* sprite)>& callback) override;
		void TraverseSprite(const std::function<void(IGalSprite* sprite)>& callback) override;
		void AddSpriteLayer(const String& layer) override;
		ISceneSpriteLayer* GetSpriteLayer(const String& layer) override;
	private:
		void Initialize();
	private:
		std::deque<SpriteLayer>  m_SpriteLayers;
		std::unordered_map<String, int> m_SpriteLayerIndexer;
	};

}
