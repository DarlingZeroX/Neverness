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
	class SequenceComponentRegistry;
	struct SequenceDirtyRegion;

	/// Document → visual read-model slice (Phase 7). Must not read widget state.
	class ISequenceProjection
	{
	public:
		virtual ~ISequenceProjection() = default;

		virtual void Rebuild(SequenceDocument& document, const SequenceComponentRegistry& registry) = 0;

		virtual void ApplyDirtyRegion(
			const SequenceDirtyRegion& dirty,
			SequenceDocument& document,
			const SequenceComponentRegistry& registry) = 0;
	};
}
