/*
 * IRuntimeExecutionServices — Sequence / 图运行时「窄服务」面（Phase 8 Contract）
 *
 * 中文：替代在序列节点中直接持有 IDialogueSystem / IGalGameEngine；
 * 宿主实现本接口并将指针注入 SSSequenceExecutionContext，内核仅调用此处声明的能力。
 */

#pragma once
#include "../VGGalCoreConfig.h"
#include <string>

namespace VisionGal::GalGame
{
	struct IRuntimeExecutionServices
	{
		virtual ~IRuntimeExecutionServices() = default;

		/// 中文：播一行对白；实现侧可转发到 DialogueSubsystem 或任意 Headless 后端。
		virtual void DialogueCharacterSay(const std::string& character, const std::string& text) = 0;
	};
}
