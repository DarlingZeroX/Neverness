/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Commands/AddSequenceEntryCommand.h"

#include "Document/SequenceDocument.h"

namespace VisionGal::Editor
{
	AddSequenceEntryCommand::AddSequenceEntryCommand(std::string typeNameId)
		: m_typeNameId(std::move(typeNameId))
	{
	}

	void AddSequenceEntryCommand::DoAdd(SequenceDocument& document)
	{
		document.AddEntryByTypeNameID(m_typeNameId);
		const auto seq = document.GetSequence();
		m_insertedIndex = seq->m_Sequence.empty() ? 0u : static_cast<unsigned>(seq->m_Sequence.size() - 1);
	}

	void AddSequenceEntryCommand::Execute(SequenceDocument& document)
	{
		DoAdd(document);
	}

	void AddSequenceEntryCommand::Undo(SequenceDocument& document)
	{
		document.RemoveEntries({m_insertedIndex});
	}

	void AddSequenceEntryCommand::Redo(SequenceDocument& document)
	{
		DoAdd(document);
	}

	std::string AddSequenceEntryCommand::GetDebugName() const
	{
		return "AddSequenceEntryCommand(" + m_typeNameId + ")";
	}
}
