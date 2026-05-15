/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Projection/SequenceTimelineProjection.h"

#include "DirtyRegions/SequenceDirtyRegion.h"
#include "Projection/SequenceProjectionContext.h"

namespace VisionGal::Editor
{
	void SequenceTimelineProjection::Rebuild(const SequenceProjectionContext& ctx)
	{
		(void)ctx;
	}

	void SequenceTimelineProjection::ApplyDirtyRegion(
		const SequenceDirtyRegion& dirty,
		const SequenceProjectionContext& ctx)
	{
		(void)dirty;
		(void)ctx;
	}
}
