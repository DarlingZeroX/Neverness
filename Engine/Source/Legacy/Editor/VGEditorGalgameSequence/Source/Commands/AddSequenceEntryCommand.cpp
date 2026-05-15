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
		const unsigned n = document.GetEntryCount();
		m_insertedIndex = n > 0 ? n - 1u : 0u;
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

	SequenceDocumentMutationSummary AddSequenceEntryCommand::DescribeExecutedMutation() const
	{
		SequenceDocumentMutationSummary s;
		s.StructuralChange = true;
		s.TouchedIndices.push_back(m_insertedIndex);
		return s;
	}
}
