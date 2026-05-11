/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include <memory>
#include <string>

namespace VisionGal::Editor
{
	class SequenceDocument;
	class SequenceDocumentViewModel;
	class SequenceExecutionController;
	class SequenceSelectionModel;
	class SequenceUndoStack;
	class SequenceClipboard;
	class SequenceInspectorRegistry;
	class SequenceValidationRegistry;
	class ISequenceEditorCommand;
	struct SequenceRuntimeSnapshot;
	struct SequenceRuntimeOverlayState;

	struct SequenceEditorContext
	{
		SequenceDocument* document = nullptr;
		SequenceExecutionController* execution = nullptr;
		SequenceSelectionModel* selection = nullptr;
		SequenceUndoStack* undo = nullptr;
		SequenceClipboard* clipboard = nullptr;
		SequenceInspectorRegistry* inspectorRegistry = nullptr;

		/// Rebuilt each frame by host; list / timeline / outliner read rows from here.
		/// 由宿主每帧重建；列表 / 时间轴 / 大纲从此读取行数据。
		SequenceDocumentViewModel* documentViewModel = nullptr;

		SequenceValidationRegistry* validationRegistry = nullptr;

		/// Overlay pushed by `SequenceRuntimeObserver` after `ExecuteTo` (no widget polling).
		/// 由 `SequenceRuntimeObserver` 在 `ExecuteTo` 之后推送（Widget 不轮询）。
		const SequenceRuntimeOverlayState* runtimeOverlay = nullptr;

		/// Optional filter string from `SequenceSearchWidget` (read-only for list widgets).
		/// 来自 `SequenceSearchWidget` 的可选过滤字符串（列表控件只读）。
		const std::string* searchFilter = nullptr;

		/// Filled by host after `ExecuteTo` / toolbar play (for status UI).
		/// 宿主在 `ExecuteTo` 或工具栏播放后填入（用于状态 UI）。
		SequenceRuntimeSnapshot* lastExecutionSnapshot = nullptr;

		/// Per-row runtime execute (save + engine step); set by host editor.
		/// 逐行运行时执行（保存 + 引擎步进）；由宿主编辑器设置。
		bool (*executeToEntry)(void* userData, unsigned index) = nullptr;
		void* executeToUserData = nullptr;

		void ExecuteCommand(std::unique_ptr<ISequenceEditorCommand> command);
	};
}
