/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Projection/SequenceProjectionPipeline.h"

#include "DirtyRegions/SequenceDirtyRegion.h"
#include "DirtyRegions/SequenceDirtyRegionFlags.h"
#include "Projection/SequenceProjectionContext.h"

namespace VisionGal::Editor
{
	void SequenceProjectionPipeline::RebuildAll(const SequenceProjectionContext& ctx)
	{
		m_listProjection.Rebuild(ctx);
		m_timelineProjection.Rebuild(ctx);
		m_graphProjection.Rebuild(ctx);
	}

	void SequenceProjectionPipeline::ApplyDirtyAll(const SequenceProjectionContext& ctx, const SequenceDirtyRegion& dirty)
	{
		m_listProjection.ApplyDirtyRegion(dirty, ctx);
		m_timelineProjection.ApplyDirtyRegion(dirty, ctx);
		m_graphProjection.ApplyDirtyRegion(dirty, ctx);
	}

	void SequenceProjectionPipeline::RunProjectionPass(
		const bool firstFrame,
		const bool hasDocSignals,
		const SequenceDirtyRegion& dirty,
		const SequenceProjectionContext& ctx)
	{
		if (!firstFrame && !hasDocSignals)
			return;

		if (firstFrame)
		{
			RebuildAll(ctx);
			return;
		}

		const bool structural =
			(dirty.Flags & SequenceDirtyRegionFlags::Structure) != SequenceDirtyRegionFlags::None;

		if (structural)
			RebuildAll(ctx);
		else
			ApplyDirtyAll(ctx, dirty);
	}
}
