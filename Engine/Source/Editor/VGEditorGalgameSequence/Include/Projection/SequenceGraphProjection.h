/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "Projection/Graph/SequenceGraphReadModel.h"
#include "Projection/ISequenceProjection.h"

#include <vector>

namespace VisionGal::Editor
{
	class SequenceGraphProjection final : public ISequenceProjection
	{
	public:
		void Rebuild(SequenceDocument& document, const SequenceComponentRegistry& registry) override;

		void ApplyDirtyRegion(
			const SequenceDirtyRegion& dirty,
			SequenceDocument& document,
			const SequenceComponentRegistry& registry) override;

		[[nodiscard]] const std::vector<SequenceGraphNodeVM>& GetNodes() const { return m_nodes; }
		[[nodiscard]] const std::vector<SequenceGraphEdgeVM>& GetEdges() const { return m_edges; }

	private:
		std::vector<SequenceGraphNodeVM> m_nodes;
		std::vector<SequenceGraphEdgeVM> m_edges;
	};
}
