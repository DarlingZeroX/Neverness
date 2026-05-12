/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include <cstdint>
#include <vector>

#include "DirtyRegions/SequenceDirtyRegionFlags.h"
#include "Transactions/SequenceTransactionTypes.h"

namespace VisionGal::Editor
{
	struct SequenceDocumentMutationSummary;

	/// Normalized dirty state for presentation scheduling (Phase 6).
	struct SequenceDirtyRegion
	{
		SequenceDirtyRegionFlags Flags = SequenceDirtyRegionFlags::None;
		std::vector<unsigned> Entries;
		bool StructureChanged = false;
		bool SearchIndexDirty = false;
		bool ValidationDirty = false;
		bool TimelineDirty = false;
		bool OutlinerDirty = false;

		void Reset();
		void Merge(const SequenceDirtyRegion& other);
	};

	[[nodiscard]] SequenceDirtyRegion BuildDirtyRegionFromMutationSummary(const SequenceDocumentMutationSummary& summary);
	[[nodiscard]] SequenceDirtyRegion BuildDirtyRegionFromTransaction(const SequenceTransaction& tx);
}
