/*
 * IDialogueSubsystem — 对白与对话流程（由 ISubsystemBus::Dialogue() 访问）
 *
 * 委托既有 IDialogueSystem；脚本与序列应通过本接口而非直接持有引擎。
 */

#pragma once
#include "IGameSystem.h"
#include "../VGGalCoreConfig.h"

namespace VisionGal::GalGame
{
	struct  IDialogueSubsystem
	{
		virtual ~IDialogueSubsystem() = default;

		virtual IDialogueSystem* GetDialogueSystem() = 0;
	};
}
