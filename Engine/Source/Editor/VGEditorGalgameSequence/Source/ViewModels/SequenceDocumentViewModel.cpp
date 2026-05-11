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
#include "Runtime/SequenceRuntimeOverlayState.h"
#include "Validation/SequenceValidationRegistry.h"
#include "ViewModels/SequenceSearchViewModel.h"

#include "VGGalgameScriptSequence/Include/Sequence/Components.h"
#include "VGGalgameScriptSequence/Interface/IVGSSequenceComponent.h"

namespace VisionGal::Editor
{
	namespace
	{
		std::string BuildSubtitle(const Ref<VisionGal::IVGSSequenceComponent>& entry)
		{
			if (entry == nullptr)
				return {};
			if (auto* d = dynamic_cast<VisionGal::VGSSC_CommonDialogue*>(entry.get()))
			{
				if (!d->DialogueCharacterName.empty())
					return d->DialogueCharacterName + " — " + d->DialogueText;
				return d->DialogueText;
			}
			if (auto* f = dynamic_cast<VisionGal::VGSSC_ChangeFigure*>(entry.get()))
				return f->TextureResourcePath;
			if (auto* b = dynamic_cast<VisionGal::VGSSC_ChangeBackground*>(entry.get()))
				return b->TextureResourcePath;
			return {};
		}
	}

	void SequenceDocumentViewModel::Rebuild(SequenceDocument& document, const SequenceComponentRegistry& registry)
	{
		m_storage.clear();
		m_visibleRows.clear();
		m_validationIssues.clear();

		const auto seq = document.GetSequence();
		unsigned i = 0;
		for (const auto& entry : seq->m_Sequence)
		{
			SequenceEntryViewModel row;
			row.EntryIndex = i;
			if (entry != nullptr)
				row.TypeNameID = entry->GetTypeNameID();
			if (const SequenceComponentMetadata* meta = registry.Find(row.TypeNameID))
			{
				row.DisplayName = meta->PrimaryLabel();
				row.Category = meta->Category;
				row.Icon = meta->Icon;
			}
			else
				row.DisplayName = row.TypeNameID;
			row.Subtitle = BuildSubtitle(entry);
			m_storage.push_back(std::move(row));
			++i;
		}
		m_visibleRows = m_storage;
	}

	void SequenceDocumentViewModel::SyncVisibleWithStorage()
	{
		for (auto& vis : m_visibleRows)
		{
			if (vis.EntryIndex >= m_storage.size())
				continue;
			const SequenceEntryViewModel& src = m_storage[vis.EntryIndex];
			vis.HasValidationError = src.HasValidationError;
			vis.RuntimeHighlight = src.RuntimeHighlight;
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
		m_visibleRows.reserve(m_storage.size());
		for (const auto& row : m_storage)
		{
			if (search.RowPassesFilters(row))
				m_visibleRows.push_back(row);
		}
	}

	void SequenceDocumentViewModel::ApplyValidation(const SequenceValidationRegistry& registry, const SequenceDocument& document)
	{
		m_validationIssues = registry.RunAll(document);
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

	void SequenceDocumentViewModel::ApplyRuntimeOverlay(const SequenceRuntimeOverlayState& overlay)
	{
		for (auto& row : m_storage)
		{
			row.RuntimeHighlight = overlay.ShowExecutionLine && row.EntryIndex == overlay.HighlightIndex;
		}
		SyncVisibleWithStorage();
	}
}
