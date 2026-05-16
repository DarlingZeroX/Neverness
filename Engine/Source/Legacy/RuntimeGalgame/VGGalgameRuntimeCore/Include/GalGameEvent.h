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
#include <NNCore/Include/Event/HEventDelegate.h>
#include <cstdint>

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

	/// 中文：与 **GalGameScriptEvent** 区分；表达引擎/协调器级生命周期，供 Editor / Debugger 订阅。
	enum class GalRuntimeLifecycleKind : std::uint8_t
	{
		None = 0,
		/// 中文：**GalRuntimeCoordinator::ResetRuntime** 已写完 **GalGameRuntimeState** 与场景清理后的同步点（仍在 **Running** 阶段内）。
		ResetCompleted = 1,
	};

	struct GalRuntimeLifecycleEvent
	{
		GalRuntimeLifecycleKind kind = GalRuntimeLifecycleKind::None;
	};

	struct GalEngineEventBus
	{
		Horizon::HEventDelegate<const GalGameScriptEvent&> OnStoryScriptEvent;
		Horizon::HEventDelegate<const GalGameScriptExecuteEvent&> OnStoryScriptExecuteEvent;

		/// 中文：Phase 8F — 宿主级生命周期（**ResetRuntime** 完成等）；与脚本 **OnStoryScriptEvent** 分离，避免调试器误绑脚本加载。
		Horizon::HEventDelegate<const GalRuntimeLifecycleEvent&> OnRuntimeLifecycleEvent;
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
