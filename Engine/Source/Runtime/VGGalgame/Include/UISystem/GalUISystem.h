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
#include "../../VGGalgameConfig.h"
#include "VGGalgameCore/Interface/IGameEngine.h"
#include "VGGalgameCore/Include/GalGameContext.h"

namespace VisionGal::GalGame
{
	class VG_GALGAME_API GalGameUISystem : public IGalGameUISystem
	{
	public:
		GalGameUISystem();
		~GalGameUISystem() override = default;
		GalGameUISystem(const GalGameUISystem&) = delete;
		GalGameUISystem& operator=(const GalGameUISystem&) = delete;
		GalGameUISystem(GalGameUISystem&&) noexcept = default;
		GalGameUISystem& operator=(GalGameUISystem&&) noexcept = default;

		void Initialize(const Ref<GalGameContext>& galCtx, IGameEngineContext* context);

		// 剧情选择UI
		void ShowChoiceUI(const std::string& id, const std::vector<std::string>& options) override;
		std::string GetChoiceOptionByIndex(int index) override;
		int GetChoiceOptionSize() const override;
		void SelectCurrentChoice(int index) override;

		// 全屏文字UI
		void ShowFullScreenTextUI(const std::vector<std::string>& texts) override;
		std::string GetFullScreenTextItem(int index) override;
		int GetFullScreenTextSize() const override;

		// 玩家输入UI
		void ShowInputUI(const std::string& id, const std::string& title, const std::string& button) override;
		void InputSubmitted(const std::string& text) override;
		std::string GetInputTitle() override;
		std::string GetInputButtonText() override;
	private:
		IScene* m_Scene = nullptr;

		Ref<GalGameContext> m_GalCtx;
		IGameEngineContext* m_GECtx;

		std::string m_CurrentChoiceID;
		std::vector<std::string> m_CurrentChoiceOptions;
		std::vector<std::string> m_CurrentFullScreenTexts;

		std::string m_CurrentInputID;
		std::string m_CurrentInputTitle;
		std::string m_CurrentInputButtonText;
	};
}
