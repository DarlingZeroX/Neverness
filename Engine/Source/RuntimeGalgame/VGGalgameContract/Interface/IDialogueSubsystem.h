/*
 * IDialogueSubsystem — 对白与对话流程（Phase 8：Contract）
 *
 * 委托既有 IDialogueSystem；脚本与序列应通过本接口而非直接持有引擎。
 * IDialogueSystem 定义见 VGGalgameRuntimeCore `IGameSystem.h`。
 */

#pragma once
#include "../VGGalCoreConfig.h"

namespace VisionGal::GalGame
{
	struct IDialogueSystem;

	struct IDialogueSubsystem
	{
		virtual ~IDialogueSubsystem() = default;

		virtual IDialogueSystem* GetDialogueSystem() = 0;
	};
}
