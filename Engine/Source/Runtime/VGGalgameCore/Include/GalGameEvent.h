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
#include <HCore/Include/Event/HEventDelegate.h>

namespace VisionGal::GalGame
{
	enum class GalGameScriptEventType
	{
		None = 0,
		OnScriptStartLoad,
		OnScriptFinishedLoad
	};

	struct GalGameScriptEvent
	{
		GalGameScriptEventType EventType = GalGameScriptEventType::None;
		std::string scriptPath;
	};

	enum class GalGameScriptExecuteEventType
	{
		None = 0,
		ContinueExecute
	};

	struct GalGameScriptExecuteEvent
	{
		GalGameScriptExecuteEventType EventType = GalGameScriptExecuteEventType::None;
	};

	struct GalEngineEventBus
	{
		Horizon::HEventDelegate<const GalGameScriptEvent&> OnStoryScriptEvent;
		Horizon::HEventDelegate<const GalGameScriptExecuteEvent&> OnStoryScriptExecuteEvent;
	};

	struct GalGameUIEvent
	{
		enum class Type
		{
			None = 0,
			ShowChoiceUI,
			ChoiceSelected,
			ShowInputUI,
			InputSubmitted,
		};

		Type EventType = Type::None;
		// 选择
		std::vector<std::string> ChoiceOptions;
		std::string ChoiceID;
		int CurrentChoiceIndex;
		// 输入
		std::string InputID;
		std::string InputTitle;
		std::string InputButtonText;
		std::string CurrentInputText;
	};

	struct GalGameUIEventBus
	{
		Horizon::HEventDelegate<const GalGameUIEvent&> OnUIEvent;
	};
}
