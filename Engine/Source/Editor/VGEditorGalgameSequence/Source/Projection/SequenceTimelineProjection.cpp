/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Projection/SequenceTimelineProjection.h"

#include "ComponentRegistry/SequenceComponentRegistry.h"
#include "DirtyRegions/SequenceDirtyRegion.h"
#include "Document/SequenceDocument.h"

namespace VisionGal::Editor
{
	void SequenceTimelineProjection::Rebuild(SequenceDocument& document, const SequenceComponentRegistry& registry)
	{
		(void)document;
		(void)registry;
	}

	void SequenceTimelineProjection::ApplyDirtyRegion(
		const SequenceDirtyRegion& dirty,
		SequenceDocument& document,
		const SequenceComponentRegistry& registry)
	{
		(void)dirty;
		(void)document;
		(void)registry;
	}
}
