/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Transactions/Patches/SequencePatchApplier.h"

#include "DirtyRegions/SequenceDirtyRegionFlags.h"

#include <variant>

namespace VisionGal::Editor
{
	SequenceDirtyRegion BuildDirtyRegionFromPatchTransaction(const SequencePatchTransactionV2& tx)
	{
		SequenceDirtyRegion region;
		for (const SequenceDocumentPatch& p : tx.Patches)
		{
			if (std::holds_alternative<SequenceInsertEntryPatch>(p) || std::holds_alternative<SequenceRemoveEntryPatch>(p)
				|| std::holds_alternative<SequenceMoveEntryPatch>(p))
			{
				region.Flags = region.Flags | SequenceDirtyRegionFlags::Structure;
				region.StructureChanged = true;
			}
			else if (const auto* sp = std::get_if<SequenceSetPropertyPatch>(&p))
			{
				region.Flags = region.Flags | SequenceDirtyRegionFlags::Property;
				region.Entries.push_back(sp->EntryIndex);
			}
		}
		return region;
	}
}
