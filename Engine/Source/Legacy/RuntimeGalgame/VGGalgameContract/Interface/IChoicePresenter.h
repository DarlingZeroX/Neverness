/*
 * IChoicePresenter — 选项 UI 表现契约（Phase 8 占位）
 *
 * 中文：与 IDialoguePresenter 对称，供 ChoiceRuntime 发事件后由 UI 实现；当前无纯虚方法，避免破坏现有构建。
 */

#pragma once

namespace VisionGal::GalGame
{
	struct IChoicePresenter
	{
		virtual ~IChoicePresenter() = default;
	};
}
