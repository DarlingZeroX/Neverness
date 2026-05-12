/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include <memory>
#include <vector>

namespace VisionGal::Editor
{
	class SequenceDocument;
	class ISequenceEditorCommand;
	struct SequenceMutationBatch;

	class SequenceUndoStack
	{
	public:
		void ExecuteCommand(std::unique_ptr<ISequenceEditorCommand> command, SequenceDocument& document);

		/// Phase 9：统一压栈入口。单条命令时保留 `TryMergeWith`；多条时包装为 `CompoundSequenceCommand`。
		void ExecuteBatch(SequenceMutationBatch&& batch, SequenceDocument& document);

		void Undo(SequenceDocument& document);
		void Redo(SequenceDocument& document);

		void Clear();

		bool CanUndo() const { return !m_undo.empty(); }
		bool CanRedo() const { return !m_redo.empty(); }

		/// Top of undo stack after `ExecuteCommand` (the command just executed).
		[[nodiscard]] const ISequenceEditorCommand* PeekUndoTop() const;

	private:
		std::vector<std::unique_ptr<ISequenceEditorCommand>> m_undo;
		std::vector<std::unique_ptr<ISequenceEditorCommand>> m_redo;
	};
}
