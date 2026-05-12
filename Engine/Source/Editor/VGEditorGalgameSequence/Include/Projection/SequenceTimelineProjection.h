/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "Projection/ISequenceProjection.h"

namespace VisionGal::Editor
{
	class SequenceTimelineProjection final : public ISequenceProjection
	{
	public:
		void Rebuild(SequenceDocument& document, const SequenceComponentRegistry& registry) override;

		void ApplyDirtyRegion(
			const SequenceDirtyRegion& dirty,
			SequenceDocument& document,
			const SequenceComponentRegistry& registry) override;
	};
}
