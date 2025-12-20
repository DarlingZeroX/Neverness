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
#include "../Interface/IGalGameEngine.h"
#include "../../EngineConfig.h"
#include "../Core/GalGameContext.h"

namespace VisionGal::GalGame
{
	class VG_ENGINE_API GalGameUISystem
	{
	public:
		GalGameUISystem();
		~GalGameUISystem() = default;
		GalGameUISystem(const GalGameUISystem&) = delete;
		GalGameUISystem& operator=(const GalGameUISystem&) = delete;
		GalGameUISystem(GalGameUISystem&&) noexcept = default;
		GalGameUISystem& operator=(GalGameUISystem&&) noexcept = default;

		void Initialize(const Ref<GalGameContext>& galCtx, IGameEngineContext* context);

		void ShowChoiceUI(const std::string& name, const std::vector<std::string>& options);

		std::string GetChoiceOptionByIndex(int index);
		int GetChoiceOptionSize() const;
		void SelectCurrentChoice(int index);
	private:
		IScene* m_Scene = nullptr;

		Ref<GalGameContext> m_GalCtx;
		IGameEngineContext* m_GECtx;

		std::vector<std::string> m_CurrentChoiceOptions;
	};


}
