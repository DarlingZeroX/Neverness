/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Commands/RemoveSequenceEntryCommand.h"

#include "Document/SequenceDocument.h"

#include "VGGalgameScriptSequence/Interface/IVGSSequenceComponent.h"

#include <algorithm>

namespace VisionGal::Editor
{
	RemoveSequenceEntryCommand::RemoveSequenceEntryCommand(std::vector<unsigned> indices)
		: m_indices(std::move(indices))
	{
		std::sort(m_indices.begin(), m_indices.end());
		m_indices.erase(std::unique(m_indices.begin(), m_indices.end()), m_indices.end());
	}

	void RemoveSequenceEntryCommand::CaptureSnapshotIfNeeded(SequenceDocument& document)
	{
		if (m_hasSnapshot)
			return;
		const auto seq = document.GetSequence();
		m_undoPairs.clear();
		for (unsigned idx : m_indices)
		{
			if (idx >= seq->m_Sequence.size() || seq->m_Sequence[idx] == nullptr)
				continue;
			m_undoPairs.push_back({idx, seq->m_Sequence[idx]->Clone()});
		}
		m_hasSnapshot = true;
	}

	void RemoveSequenceEntryCommand::Execute(SequenceDocument& document)
	{
		CaptureSnapshotIfNeeded(document);
		document.RemoveEntries(m_indices);
	}

	void RemoveSequenceEntryCommand::Undo(SequenceDocument& document)
	{
		for (auto it = m_undoPairs.rbegin(); it != m_undoPairs.rend(); ++it)
			document.InsertEntryAt(it->first, it->second);
	}

	void RemoveSequenceEntryCommand::Redo(SequenceDocument& document)
	{
		document.RemoveEntries(m_indices);
	}

	std::string RemoveSequenceEntryCommand::GetDebugName() const
	{
		return "RemoveSequenceEntryCommand";
	}
}
