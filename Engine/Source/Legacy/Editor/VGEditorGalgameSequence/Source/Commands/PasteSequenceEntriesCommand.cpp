/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Commands/PasteSequenceEntriesCommand.h"

#include "Document/SequenceDocument.h"

#include "VGGalgameSequenceRuntime/Interface/IVGSSequenceComponent.h"

namespace VisionGal::Editor
{
	PasteSequenceEntriesCommand::PasteSequenceEntriesCommand(unsigned insertIndex, std::vector<Ref<VisionGal::IVGSSequenceComponent>> prototypes)
		: m_insertIndex(insertIndex)
		, m_prototypes(std::move(prototypes))
	{
	}

	void PasteSequenceEntriesCommand::DoInsert(SequenceDocument& document)
	{
		unsigned at = m_insertIndex;
		for (const auto& proto : m_prototypes)
		{
			if (proto == nullptr)
				continue;
			Ref<VisionGal::IVGSSequenceComponent> c = proto->Clone();
			document.InsertEntryAt(at, std::move(c));
			++at;
		}
		m_insertedCount = static_cast<unsigned>(m_prototypes.size());
	}

	void PasteSequenceEntriesCommand::Execute(SequenceDocument& document)
	{
		DoInsert(document);
	}

	void PasteSequenceEntriesCommand::Undo(SequenceDocument& document)
	{
		if (m_insertedCount == 0)
			return;
		std::vector<unsigned> indices;
		indices.reserve(m_insertedCount);
		for (unsigned i = 0; i < m_insertedCount; ++i)
			indices.push_back(m_insertIndex + i);
		document.RemoveEntries(indices);
	}

	void PasteSequenceEntriesCommand::Redo(SequenceDocument& document)
	{
		DoInsert(document);
	}

	std::string PasteSequenceEntriesCommand::GetDebugName() const
	{
		return "PasteSequenceEntriesCommand(" + std::to_string(m_prototypes.size()) + ")";
	}

	SequenceDocumentMutationSummary PasteSequenceEntriesCommand::DescribeExecutedMutation() const
	{
		SequenceDocumentMutationSummary s;
		s.StructuralChange = true;
		for (unsigned i = 0; i < m_insertedCount; ++i)
			s.TouchedIndices.push_back(m_insertIndex + i);
		return s;
	}
}
