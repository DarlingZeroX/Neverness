/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

namespace VisionGal::Editor
{
	class SequenceDocument;
	class SequenceDocumentViewModel;
	class SequenceComponentRegistry;
	struct SequenceDirtyRegion;

	/// Document → visual structure slice. Must not read widget state (Phase 6).
	class ISequenceProjection
	{
	public:
		virtual ~ISequenceProjection() = default;

		virtual void Apply(
			bool seedPresentation,
			const SequenceDirtyRegion& dirty,
			SequenceDocument& document,
			SequenceDocumentViewModel& viewModel,
			SequenceComponentRegistry& registry) = 0;
	};
}
