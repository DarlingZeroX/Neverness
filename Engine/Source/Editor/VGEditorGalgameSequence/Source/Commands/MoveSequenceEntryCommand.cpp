/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Commands/MoveSequenceEntryCommand.h"

#include "Document/SequenceDocument.h"

#include <algorithm>

namespace VisionGal::Editor
{
	MoveSequenceEntryCommand::MoveSequenceEntryCommand(unsigned sourceIndex, unsigned targetIndex)
		: m_source(sourceIndex)
		, m_target(targetIndex)
	{
	}

	void MoveSequenceEntryCommand::DoReorder(SequenceDocument& document)
	{
		if (m_source > m_target)
			document.ReorderInsertBefore(m_source, m_target);
		else
			document.ReorderInsertBehind(m_source, m_target);
	}

	void MoveSequenceEntryCommand::Execute(SequenceDocument& document)
	{
		m_beforeOrder = document.CopyEntryRefVector();
		DoReorder(document);
	}

	void MoveSequenceEntryCommand::Undo(SequenceDocument& document)
	{
		document.SetSequenceEntries(m_beforeOrder);
	}

	void MoveSequenceEntryCommand::Redo(SequenceDocument& document)
	{
		DoReorder(document);
	}

	std::string MoveSequenceEntryCommand::GetDebugName() const
	{
		return "MoveSequenceEntryCommand";
	}

	SequenceDocumentMutationSummary MoveSequenceEntryCommand::DescribeExecutedMutation() const
	{
		SequenceDocumentMutationSummary s;
		s.StructuralChange = true;
		const unsigned lo = (std::min)(m_source, m_target);
		const unsigned hi = (std::max)(m_source, m_target);
		for (unsigned i = lo; i <= hi; ++i)
			s.TouchedIndices.push_back(i);
		return s;
	}
}
