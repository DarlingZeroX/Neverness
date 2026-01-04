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

		// 剧情选择UI
		void ShowChoiceUI(const std::string& name, const std::vector<std::string>& options);
		std::string GetChoiceOptionByIndex(int index);
		int GetChoiceOptionSize() const;
		void SelectCurrentChoice(int index);

		// 全屏文字UI
		void ShowFullScreenTextUI(const std::vector<std::string>& texts);
		std::string GetFullScreenTextItem(int index);
		int GetFullScreenTextSize() const;

		// 玩家输入UI
		void ShowInputUI(const std::string& id, const std::string& title, const std::string& button);
		void InputSubmitted(const std::string& text);
		std::string GetInputTitle();
		std::string GetInputButtonText();
	private:
		IScene* m_Scene = nullptr;

		Ref<GalGameContext> m_GalCtx;
		IGameEngineContext* m_GECtx;

		std::vector<std::string> m_CurrentChoiceOptions;
		std::vector<std::string> m_CurrentFullScreenTexts;

		std::string m_InputID;
		std::string m_InputTitle;
		std::string m_InputButtonText;
	};
}
