/*
 * IDialoguePresenter — 对白表现层契约（Phase 8）
 *
 * 中文：Runtime 侧只应通过事件或本接口的窄方法驱动 UI；具体 Rml / 动画实现由 VGGalgame / Presentation 提供。
 * 当前为占位接口，便于后续将 DialogueRmlPresentation 迁入独立模块并实现本接口。
 */

#pragma once

namespace VisionGal::GalGame
{
	struct IDialoguePresenter
	{
		virtual ~IDialoguePresenter() = default;

		/// 中文：对白行变更后通知表现层刷新（可选；未实现可为空操作）。
		virtual void OnDialogueVisualChanged() {}
	};
}
