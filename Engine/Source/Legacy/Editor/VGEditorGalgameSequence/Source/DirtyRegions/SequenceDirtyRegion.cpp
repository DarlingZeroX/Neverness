/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "DirtyRegions/SequenceDirtyRegion.h"

#include "Events/SequenceEditorEvent.h"

#include <algorithm>
#include <unordered_set>

namespace VisionGal::Editor
{
	void SequenceDirtyRegion::Reset()
	{
		Flags = SequenceDirtyRegionFlags::None;
		Entries.clear();
		StructureChanged = false;
		SearchIndexDirty = false;
		ValidationDirty = false;
		TimelineDirty = false;
		OutlinerDirty = false;
	}

	void SequenceDirtyRegion::Merge(const SequenceDirtyRegion& other)
	{
		Flags = Flags | other.Flags;
		Entries.insert(Entries.end(), other.Entries.begin(), other.Entries.end());
		std::sort(Entries.begin(), Entries.end());
		Entries.erase(std::unique(Entries.begin(), Entries.end()), Entries.end());
		StructureChanged = StructureChanged || other.StructureChanged;
		SearchIndexDirty = SearchIndexDirty || other.SearchIndexDirty;
		ValidationDirty = ValidationDirty || other.ValidationDirty;
		TimelineDirty = TimelineDirty || other.TimelineDirty;
		OutlinerDirty = OutlinerDirty || other.OutlinerDirty;
	}

	SequenceDirtyRegion BuildDirtyRegionFromMutationSummary(const SequenceDocumentMutationSummary& summary)
	{
		SequenceDirtyRegion d;
		if (summary.StructuralChange)
		{
			d.Flags = SequenceDirtyRegionFlags::Structure | SequenceDirtyRegionFlags::SearchIndex
				| SequenceDirtyRegionFlags::Validation | SequenceDirtyRegionFlags::TimelineLayout
				| SequenceDirtyRegionFlags::OutlinerLayout;
			d.StructureChanged = true;
			d.SearchIndexDirty = true;
			d.ValidationDirty = true;
			d.TimelineDirty = true;
			d.OutlinerDirty = true;
		}
		else if (!summary.TouchedIndices.empty())
		{
			d.Flags |= SequenceDirtyRegionFlags::Property | SequenceDirtyRegionFlags::Validation
				| SequenceDirtyRegionFlags::SearchIndex | SequenceDirtyRegionFlags::TimelineLayout
				| SequenceDirtyRegionFlags::OutlinerLayout;
			d.ValidationDirty = true;
			d.SearchIndexDirty = true;
			d.TimelineDirty = true;
			d.OutlinerDirty = true;
		}
		std::unordered_set<unsigned> uniq(summary.TouchedIndices.begin(), summary.TouchedIndices.end());
		d.Entries.assign(uniq.begin(), uniq.end());
		std::sort(d.Entries.begin(), d.Entries.end());
		return d;
	}

	SequenceDirtyRegion BuildDirtyRegionFromTransaction(const SequenceTransaction& tx)
	{
		SequenceDirtyRegion d;
		for (const SequenceMutationRecord& m : tx.Mutations)
		{
			if (m.Type == SequenceMutationType::Structure)
			{
				d.Flags |= SequenceDirtyRegionFlags::Structure | SequenceDirtyRegionFlags::SearchIndex
					| SequenceDirtyRegionFlags::Validation | SequenceDirtyRegionFlags::TimelineLayout
					| SequenceDirtyRegionFlags::OutlinerLayout;
				d.StructureChanged = true;
				d.SearchIndexDirty = true;
				d.ValidationDirty = true;
				d.TimelineDirty = true;
				d.OutlinerDirty = true;
			}
			else if (m.Type == SequenceMutationType::EntryProperty && m.EntryIndex >= 0)
			{
				d.Flags |= SequenceDirtyRegionFlags::Property | SequenceDirtyRegionFlags::Validation
					| SequenceDirtyRegionFlags::SearchIndex | SequenceDirtyRegionFlags::TimelineLayout
					| SequenceDirtyRegionFlags::OutlinerLayout;
				d.ValidationDirty = true;
				d.SearchIndexDirty = true;
				d.TimelineDirty = true;
				d.OutlinerDirty = true;
				d.Entries.push_back(static_cast<unsigned>(m.EntryIndex));
			}
		}
		std::sort(d.Entries.begin(), d.Entries.end());
		d.Entries.erase(std::unique(d.Entries.begin(), d.Entries.end()), d.Entries.end());
		return d;
	}
}
