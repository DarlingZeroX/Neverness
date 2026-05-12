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
	class SequenceAuthoringGraph;

	class SequenceGraphProjection final : public ISequenceProjection
	{
	public:
		void SetAuthoringGraph(SequenceAuthoringGraph* graph) { m_authoringGraph = graph; }

		void Rebuild(const SequenceProjectionContext& ctx) override;

		void ApplyDirtyRegion(const SequenceDirtyRegion& dirty, const SequenceProjectionContext& ctx) override;

		[[nodiscard]] const std::vector<SequenceGraphNodeVM>& GetNodes() const { return m_nodes; }
		[[nodiscard]] const std::vector<SequenceGraphEdgeVM>& GetEdges() const { return m_edges; }

	private:
		SequenceAuthoringGraph* m_authoringGraph = nullptr;
		std::vector<SequenceGraphNodeVM> m_nodes;
		std::vector<SequenceGraphEdgeVM> m_edges;
	};
}
