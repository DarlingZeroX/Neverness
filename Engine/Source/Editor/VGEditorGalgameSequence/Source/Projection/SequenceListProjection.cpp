/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Projection/SequenceListProjection.h"

#include "ComponentRegistry/SequenceComponentRegistry.h"
#include "DirtyRegions/SequenceDirtyRegion.h"
#include "Document/SequenceDocument.h"
#include "ViewModels/SequenceDocumentViewModel.h"

namespace VisionGal::Editor
{
	void SequenceListProjection::Apply(
		const bool seedPresentation,
		const SequenceDirtyRegion& dirty,
		SequenceDocument& document,
		SequenceDocumentViewModel& viewModel,
		SequenceComponentRegistry& registry)
	{
		if (seedPresentation)
		{
			viewModel.Rebuild(document, registry);
			return;
		}

		const bool structural =
			(dirty.Flags & SequenceDirtyRegionFlags::Structure) != SequenceDirtyRegionFlags::None;
		const bool property =
			(dirty.Flags & SequenceDirtyRegionFlags::Property) != SequenceDirtyRegionFlags::None;

		if (structural)
		{
			viewModel.Rebuild(document, registry);
			return;
		}

		if (property && !dirty.Entries.empty())
		{
			std::vector<unsigned> indices = dirty.Entries;
			viewModel.RebuildEntriesAtIndices(document, registry, indices);
		}
	}
}
