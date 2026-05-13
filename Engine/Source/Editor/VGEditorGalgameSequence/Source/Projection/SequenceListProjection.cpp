/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Projection/SequenceListProjection.h"

#include "ComponentRegistry/SequenceComponentMetadata.h"
#include "ComponentRegistry/SequenceComponentRegistry.h"
#include "DirtyRegions/SequenceDirtyRegion.h"
#include "Projection/SequenceProjectionContext.h"
#include "DirtyRegions/SequenceDirtyRegionFlags.h"
#include "Document/SequenceDocument.h"
#include "Document/SequenceEntryStoragePool.h"
#include "Runtime/SequenceRuntimeOverlayState.h"
#include "Validation/SequenceValidationIssue.h"

#include "VGGalgameSequenceRuntime/Include/Sequence/Components.h"
#include "VGGalgameSequenceRuntime/Interface/IVGSSequenceComponent.h"

#include <unordered_set>

namespace VisionGal::Editor
{
	namespace
	{
		const SequenceEntryStoragePool g_entryRowPool;
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

	void SequenceListProjection::Rebuild(const SequenceProjectionContext& ctx)
	{
		if (ctx.document == nullptr || ctx.registry == nullptr)
			return;
		SequenceDocument& document = *ctx.document;
		const SequenceComponentRegistry& registry = *ctx.registry;
		const unsigned count = document.GetEntryCount();
		g_entryRowPool.PrepareRowVector(m_entryRows, count);
		constexpr unsigned kChunk = 2048;
		for (unsigned base = 0; base < count; base += kChunk)
		{
			const unsigned end = (base + kChunk < count) ? (base + kChunk) : count;
			for (unsigned i = base; i < end; ++i)
				m_entryRows.push_back(BuildRow(i, document, registry));
		}
	}

	void SequenceListProjection::ApplyDirtyRegion(
		const SequenceDirtyRegion& dirty,
		const SequenceProjectionContext& ctx)
	{
		if (ctx.document == nullptr || ctx.registry == nullptr)
			return;
		SequenceDocument& document = *ctx.document;
		const SequenceComponentRegistry& registry = *ctx.registry;
		const bool structural =
			(dirty.Flags & SequenceDirtyRegionFlags::Structure) != SequenceDirtyRegionFlags::None;
		const bool property =
			(dirty.Flags & SequenceDirtyRegionFlags::Property) != SequenceDirtyRegionFlags::None;

		if (structural)
		{
			Rebuild(ctx);
			return;
		}

		if (property && !dirty.Entries.empty())
		{
			if (m_entryRows.size() != document.GetEntryCount())
			{
				Rebuild(ctx);
				return;
			}
			std::unordered_set<unsigned> uniq(dirty.Entries.begin(), dirty.Entries.end());
			for (unsigned i : uniq)
			{
				if (i >= m_entryRows.size())
					continue;
				m_entryRows[i] = BuildRow(i, document, registry);
			}
		}
	}

	void SequenceListProjection::ApplyValidationIssues(const std::vector<SequenceValidationIssue>& issues)
	{
		for (auto& row : m_entryRows)
			row.HasValidationError = false;
		for (const auto& issue : issues)
		{
			if (issue.Severity == SequenceValidationSeverity::Info)
				continue;
			if (issue.EntryIndex >= m_entryRows.size())
				continue;
			m_entryRows[issue.EntryIndex].HasValidationError = true;
		}
	}

	void SequenceListProjection::ApplyRuntimeOverlay(const SequenceRuntimeOverlayState& overlay)
	{
		for (auto& row : m_entryRows)
		{
			row.RuntimeHighlight = overlay.ShowExecutionLine && row.EntryIndex == overlay.HighlightIndex;
			row.EntryBreakpoint = false;
		}
		for (const uint32_t bi : overlay.BreakpointIndices)
		{
			if (bi < m_entryRows.size())
				m_entryRows[static_cast<size_t>(bi)].EntryBreakpoint = true;
		}
	}
}
