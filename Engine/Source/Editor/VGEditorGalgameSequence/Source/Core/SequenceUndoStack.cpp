/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Core/SequenceUndoStack.h"

#include "Commands/CompoundSequenceCommand.h"
#include "Commands/ISequenceEditorCommand.h"
#include "Document/SequenceDocument.h"
#include "Transactions/Pipeline/SequenceMutationBatch.h"

namespace VisionGal::Editor
{
	void SequenceUndoStack::ExecuteCommand(std::unique_ptr<ISequenceEditorCommand> command, SequenceDocument& document)
	{
		if (command == nullptr)
			return;
		if (!m_undo.empty() && m_undo.back()->TryMergeWith(*command, document))
		{
			m_redo.clear();
			return;
		}
		command->Execute(document);
		m_undo.push_back(std::move(command));
		m_redo.clear();
	}

	void SequenceUndoStack::ExecuteBatch(SequenceMutationBatch&& batch, SequenceDocument& document)
	{
		if (batch.Commands.empty())
			return;
		if (batch.Commands.size() == 1)
		{
			ExecuteCommand(std::move(batch.Commands[0]), document);
			return;
		}
		auto compound = std::make_unique<CompoundSequenceCommand>(std::move(batch.Commands));
		compound->Execute(document);
		m_undo.push_back(std::move(compound));
		m_redo.clear();
	}

	void SequenceUndoStack::Undo(SequenceDocument& document)
	{
		if (m_undo.empty())
			return;
		std::unique_ptr<ISequenceEditorCommand> cmd = std::move(m_undo.back());
		m_undo.pop_back();
		cmd->Undo(document);
		m_redo.push_back(std::move(cmd));
	}

	void SequenceUndoStack::Redo(SequenceDocument& document)
	{
		if (m_redo.empty())
			return;
		std::unique_ptr<ISequenceEditorCommand> cmd = std::move(m_redo.back());
		m_redo.pop_back();
		cmd->Redo(document);
		m_undo.push_back(std::move(cmd));
	}

	void SequenceUndoStack::Clear()
	{
		m_undo.clear();
		m_redo.clear();
	}

	const ISequenceEditorCommand* SequenceUndoStack::PeekUndoTop() const
	{
		return m_undo.empty() ? nullptr : m_undo.back().get();
	}
}
