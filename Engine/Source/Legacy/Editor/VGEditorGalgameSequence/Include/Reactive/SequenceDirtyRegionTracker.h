/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "DirtyRegions/SequenceDirtyRegionFlags.h"

#include "Events/SequenceEditorEvent.h"

namespace VisionGal::Editor
{
	class SequenceDirtyRegionTracker
	{
	public:
		void Reset() { m_flags = SequenceDirtyRegionFlags::None; }

		void MergeFromMutationSummary(const SequenceDocumentMutationSummary& summary)
		{
			if (summary.StructuralChange)
				m_flags = SequenceDirtyRegionFlags::Structure | SequenceDirtyRegionFlags::SearchIndex
					| SequenceDirtyRegionFlags::Validation | SequenceDirtyRegionFlags::TimelineLayout
					| SequenceDirtyRegionFlags::OutlinerLayout;
			else if (!summary.TouchedIndices.empty())
				m_flags |= SequenceDirtyRegionFlags::Property | SequenceDirtyRegionFlags::Validation
					| SequenceDirtyRegionFlags::SearchIndex | SequenceDirtyRegionFlags::TimelineLayout
					| SequenceDirtyRegionFlags::OutlinerLayout;
		}

		[[nodiscard]] SequenceDirtyRegionFlags Flags() const { return m_flags; }

	private:
		SequenceDirtyRegionFlags m_flags = SequenceDirtyRegionFlags::None;
	};
}
