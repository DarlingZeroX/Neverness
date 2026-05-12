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
	class SequenceGraphProjection final : public ISequenceProjection
	{
	public:
		void Apply(
			bool seedPresentation,
			const SequenceDirtyRegion& dirty,
			SequenceDocument& document,
			SequenceDocumentViewModel& viewModel,
			SequenceComponentRegistry& registry) override;
	};
}
