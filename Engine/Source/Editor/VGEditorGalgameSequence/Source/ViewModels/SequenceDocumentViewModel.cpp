/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "ViewModels/SequenceDocumentViewModel.h"

#include "ComponentRegistry/SequenceComponentMetadata.h"
#include "ComponentRegistry/SequenceComponentRegistry.h"
#include "Document/SequenceDocument.h"
#include "Projection/SequenceListProjection.h"
#include "Runtime/SequenceRuntimeOverlayState.h"
#include "Services/SequenceSearchIndexService.h"
#include "Validation/SequenceValidationRegistry.h"
#include "ViewModels/SequenceSearchViewModel.h"

#include "DirtyRegions/SequenceDirtyRegion.h"
#include "DirtyRegions/SequenceDirtyRegionFlags.h"

#include "VGGalgameScriptSequence/Include/Sequence/Components.h"
#include "VGGalgameScriptSequence/Interface/IVGSSequenceComponent.h"

#include <unordered_set>

namespace VisionGal::Editor
{
	namespace
	{
		std::string BuildSubtitle(const VisionGal::IVGSSequenceComponent* entry)
		{
			if (entry == nullptr)
				return {};
			if (auto* d = dynamic_cast<const VisionGal::VGSSC_CommonDialogue*>(entry))
			{
				if (!d->DialogueCharacterName.empty())
					return d->DialogueCharacterName + " — " + d->DialogueText;
				return d->DialogueText;
			}
			if (auto* f = dynamic_cast<const VisionGal::VGSSC_ChangeFigure*>(entry))
				return f->TextureResourcePath;
			if (auto* b = dynamic_cast<const VisionGal::VGSSC_ChangeBackground*>(entry))
				return b->TextureResourcePath;
			return {};
		}

		SequenceEntryViewModel BuildRow(
			unsigned i,
			const SequenceDocument& document,
			const SequenceComponentRegistry& registry)
		{
			const VisionGal::IVGSSequenceComponent* entry = document.GetEntryAt(i);
			SequenceEntryViewModel row;
			row.EntryIndex = i;
			if (entry != nullptr)
				row.TypeNameID = const_cast<VisionGal::IVGSSequenceComponent*>(entry)->GetTypeNameID();
			if (const SequenceComponentMetadata* meta = registry.Find(row.TypeNameID))
			{
				row.DisplayName = meta->PrimaryLabel();
				row.Category = meta->Category;
				row.Icon = meta->Icon;
			}
			else
				row.DisplayName = row.TypeNameID;
			row.Subtitle = BuildSubtitle(entry);
			return row;
		}
	}

	const std::vector<SequenceEntryViewModel>& SequenceDocumentViewModel::GetEntryStorage() const
	{
		if (m_listProjection != nullptr)
			return m_listProjection->GetEntryRows();
		return m_storage;
	}

	void SequenceDocumentViewModel::Rebuild(SequenceDocument& document, const SequenceComponentRegistry& registry)
	{
		m_validationIssues.clear();

		if (m_listProjection != nullptr)
		{
			m_listProjection->Rebuild(document, registry);
			m_visibleRows = m_listProjection->GetEntryRows();
			return;
		}

		m_storage.clear();
		m_visibleRows.clear();

		const unsigned count = document.GetEntryCount();
		for (unsigned i = 0; i < count; ++i)
			m_storage.push_back(BuildRow(i, document, registry));
		m_visibleRows = m_storage;
	}

	void SequenceDocumentViewModel::RebuildEntriesAtIndices(
		SequenceDocument& document,
		const SequenceComponentRegistry& registry,
		const std::vector<unsigned>& indices)
	{
		if (m_listProjection != nullptr)
		{
			SequenceDirtyRegion d{};
			d.Flags = SequenceDirtyRegionFlags::Property;
			d.Entries = indices;
			m_listProjection->ApplyDirtyRegion(d, document, registry);
			SyncVisibleWithStorage();
			return;
		}

		if (m_storage.size() != document.GetEntryCount())
		{
			Rebuild(document, registry);
			return;
		}
		std::unordered_set<unsigned> uniq(indices.begin(), indices.end());
		for (unsigned i : uniq)
		{
			if (i >= m_storage.size())
				continue;
			m_storage[i] = BuildRow(i, document, registry);
		}
		SyncVisibleWithStorage();
	}

	void SequenceDocumentViewModel::SyncVisibleWithStorage()
	{
		const std::vector<SequenceEntryViewModel>& storage = GetEntryStorage();
		for (auto& vis : m_visibleRows)
		{
			if (vis.EntryIndex >= storage.size())
				continue;
			const SequenceEntryViewModel& src = storage[vis.EntryIndex];
			vis.HasValidationError = src.HasValidationError;
			vis.RuntimeHighlight = src.RuntimeHighlight;
			vis.EntryBreakpoint = src.EntryBreakpoint;
		}
	}

	void SequenceDocumentViewModel::ApplySearchFilter(const std::string& filter)
	{
		SequenceSearchViewModel tmp;
		tmp.TextFilter() = filter;
		tmp.SetActiveDimensions(static_cast<uint32_t>(SequenceSearchViewModel::Dimension::TextMatch));
		ApplySearchViewModel(tmp);
	}

	void SequenceDocumentViewModel::ApplySearchViewModel(const SequenceSearchViewModel& search)
	{
		m_visibleRows.clear();
		const std::vector<SequenceEntryViewModel>& storage = GetEntryStorage();
		m_visibleRows.reserve(storage.size());
		for (const auto& row : storage)
		{
			if (search.RowPassesFilters(row))
				m_visibleRows.push_back(row);
		}
	}

	void SequenceDocumentViewModel::ApplySearchViewModelWithIndex(
		const SequenceSearchIndexService& index,
		const SequenceSearchViewModel& search)
	{
		std::vector<unsigned> candidates;
		const std::vector<SequenceEntryViewModel>& storage = GetEntryStorage();
		const bool narrowByIndex = search.HasDimension(SequenceSearchViewModel::Dimension::TextMatch)
			&& !search.TextFilter().empty();
		if (narrowByIndex)
			candidates = index.QueryTextIndices(search.TextFilter());
		else
		{
			candidates.reserve(storage.size());
			for (unsigned i = 0; i < storage.size(); ++i)
				candidates.push_back(i);
		}

		m_visibleRows.clear();
		m_visibleRows.reserve(candidates.size());
		for (unsigned entryIndex : candidates)
		{
			if (entryIndex >= storage.size())
				continue;
			const SequenceEntryViewModel& row = storage[entryIndex];
			if (search.RowPassesFilters(row))
				m_visibleRows.push_back(row);
		}
	}

	void SequenceDocumentViewModel::ApplyValidationIssues(const std::vector<SequenceValidationIssue>& issues)
	{
		m_validationIssues = issues;
		if (m_listProjection != nullptr)
		{
			m_listProjection->ApplyValidationIssues(issues);
			SyncVisibleWithStorage();
			return;
		}

		for (auto& row : m_storage)
			row.HasValidationError = false;
		for (const auto& issue : m_validationIssues)
		{
			if (issue.Severity == SequenceValidationSeverity::Info)
				continue;
			if (issue.EntryIndex >= m_storage.size())
				continue;
			m_storage[issue.EntryIndex].HasValidationError = true;
		}
		SyncVisibleWithStorage();
	}

	void SequenceDocumentViewModel::ApplyValidation(const SequenceValidationRegistry& registry, const SequenceDocument& document)
	{
		ApplyValidationIssues(registry.RunAll(document));
	}

	void SequenceDocumentViewModel::ApplyRuntimeOverlay(const SequenceRuntimeOverlayState& overlay)
	{
		if (m_listProjection != nullptr)
		{
			m_listProjection->ApplyRuntimeOverlay(overlay);
			SyncVisibleWithStorage();
			return;
		}

		for (auto& row : m_storage)
		{
			row.RuntimeHighlight = overlay.ShowExecutionLine && row.EntryIndex == overlay.HighlightIndex;
			row.EntryBreakpoint = false;
		}
		for (const uint32_t bi : overlay.BreakpointIndices)
		{
			if (bi < m_storage.size())
				m_storage[static_cast<size_t>(bi)].EntryBreakpoint = true;
		}
		SyncVisibleWithStorage();
	}
}
