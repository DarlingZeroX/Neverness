/*
 * DialogueRuntime vs Presentation — 概念拆分说明（Phase 8.4）
 *
 * 中文：`DialogueSystem`（VGGalgame）当前仍同时承担：
 * - **Runtime 叙事状态**（当前对白行、Auto/FastForward、语音句柄、Continue 语义）
 * - **Presentation**（Rml 数据模型、TypingEffect、屏幕布局）
 *
 * 后续步骤：
 * - 将叙事状态迁入 `DialogueRuntime`（或等价命名）并仅依赖 Contract / RuntimeCore；
 * - 将 Rml/打字机迁入 `DialoguePresenter`，通过窄接口订阅 Runtime 事件。
 *
 * Phase 8 本提交仅落盘架构边界说明，避免在单 PR 内大规模移动实现文件。
 */

#pragma once

namespace VisionGal::GalGame::DialogueSplitPhase8
{
	// 占位命名空间：便于全局搜索 Phase 8.4 迁移入口。
}
