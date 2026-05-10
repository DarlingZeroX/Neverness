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
	class SequenceExecutionController;
	class SequenceSelectionModel;
	class SequenceUndoStack;
	class SequenceClipboard;
	class SequenceInspectorRegistry;
	class ISequenceEditorCommand;
	struct SequenceRuntimeSnapshot;

	struct SequenceEditorContext
	{
		SequenceDocument* document = nullptr;
		SequenceExecutionController* execution = nullptr;
		SequenceSelectionModel* selection = nullptr;
		SequenceUndoStack* undo = nullptr;
		SequenceClipboard* clipboard = nullptr;
		SequenceInspectorRegistry* inspectorRegistry = nullptr;

		/// Optional filter string from `SequenceSearchWidget` (read-only for list widgets).
		const std::string* searchFilter = nullptr;

		/// Filled by host after `ExecuteTo` / toolbar play (for status UI).
		SequenceRuntimeSnapshot* lastExecutionSnapshot = nullptr;

		/// Per-row runtime execute (save + engine step); set by host editor.
		bool (*executeToEntry)(void* userData, unsigned index) = nullptr;
		void* executeToUserData = nullptr;

		void ExecuteCommand(std::unique_ptr<ISequenceEditorCommand> command);
	};
}
