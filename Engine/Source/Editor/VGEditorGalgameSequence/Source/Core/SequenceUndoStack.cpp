/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Core/SequenceUndoStack.h"

#include "Commands/ISequenceEditorCommand.h"
#include "Document/SequenceDocument.h"

namespace VisionGal::Editor
{
	void SequenceUndoStack::ExecuteCommand(std::unique_ptr<ISequenceEditorCommand> command, SequenceDocument& document)
	{
		if (command == nullptr)
			return;
		command->Execute(document);
		m_undo.push_back(std::move(command));
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
