/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Services/SequenceSearchIndexService.h"

#include "DirtyRegions/SequenceDirtyRegion.h"
#include "DirtyRegions/SequenceDirtyRegionFlags.h"
#include "Document/SequenceDocument.h"
#include "Events/SequenceEditorEvent.h"

#include <cctype>

namespace VisionGal::Editor
{
	namespace
	{
		std::string ToLowerAscii(std::string s)
		{
			for (char& c : s)
				c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
			return s;
		}
	}

	void SequenceSearchIndexService::RebuildFromViewStorage(const std::vector<SequenceEntryViewModel>& rows)
	{
		m_normalizedLines.clear();
		m_normalizedLines.reserve(rows.size());
		for (const auto& row : rows)
		{
			std::string blob = row.TypeNameID + " " + row.DisplayName + " " + row.Subtitle + " " + row.Category;
			m_normalizedLines.push_back(ToLowerAscii(std::move(blob)));
		}
	}

	void SequenceSearchIndexService::RebuildFromViewStorageOrIncremental(
		const SequenceDocument& /*document*/,
		const std::vector<SequenceEntryViewModel>& rows,
		const SequenceDocumentMutationSummary& mutSummary,
		const SequenceDirtyRegion& dirty)
	{
		if (dirty.SearchIndexDirty)
		{
			RebuildFromViewStorage(rows);
			return;
		}
		const bool structural = mutSummary.StructuralChange
			|| (dirty.Flags & SequenceDirtyRegionFlags::Structure) != SequenceDirtyRegionFlags::None;
		const size_t touchCount = mutSummary.TouchedIndices.size() + dirty.Entries.size();
		if (structural || touchCount > 512 || m_normalizedLines.size() != rows.size())
		{
			RebuildFromViewStorage(rows);
			return;
		}
		if (touchCount == 0 && m_normalizedLines.size() == rows.size())
			return;
		if (m_normalizedLines.size() != rows.size())
		{
			RebuildFromViewStorage(rows);
			return;
		}
		auto rebuildLine = [&](const unsigned idx) {
			if (idx >= rows.size())
				return;
			std::string blob =
				rows[idx].TypeNameID + " " + rows[idx].DisplayName + " " + rows[idx].Subtitle + " " + rows[idx].Category;
			m_normalizedLines[idx] = ToLowerAscii(std::move(blob));
		};
		for (const unsigned idx : mutSummary.TouchedIndices)
			rebuildLine(idx);
		for (const unsigned idx : dirty.Entries)
			rebuildLine(idx);
	}

	std::vector<unsigned> SequenceSearchIndexService::QueryTextIndices(const std::string& needle) const
	{
		std::vector<unsigned> out;
		if (needle.empty())
		{
			out.reserve(m_normalizedLines.size());
			for (unsigned i = 0; i < m_normalizedLines.size(); ++i)
				out.push_back(i);
			return out;
		}
		const std::string n = ToLowerAscii(needle);
		for (unsigned i = 0; i < m_normalizedLines.size(); ++i)
		{
			if (m_normalizedLines[i].find(n) != std::string::npos)
				out.push_back(i);
		}
		return out;
	}
}
