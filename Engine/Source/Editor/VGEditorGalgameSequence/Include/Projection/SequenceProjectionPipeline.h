/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#pragma once

#include "Projection/SequenceGraphProjection.h"
#include "Projection/SequenceListProjection.h"
#include "Projection/SequenceTimelineProjection.h"

namespace VisionGal::Editor
{
	class SequenceAuthoringGraph;
	struct SequenceDirtyRegion;
	struct SequenceProjectionContext;

	/// Phase 9：单一投影管线。封装「首帧 / 结构脏 / 属性脏」策略，对 List / Timeline / Graph
	/// 统一派发；调度器不再手写三分支 list/timeline/graph 调用链。
	class SequenceProjectionPipeline
	{
	public:
		[[nodiscard]] SequenceListProjection& GetListProjection() { return m_listProjection; }
		[[nodiscard]] const SequenceListProjection& GetListProjection() const { return m_listProjection; }
		[[nodiscard]] SequenceGraphProjection& GetGraphProjection() { return m_graphProjection; }
		[[nodiscard]] SequenceTimelineProjection& GetTimelineProjection() { return m_timelineProjection; }

		void SetAuthoringGraph(SequenceAuthoringGraph* graph) { m_graphProjection.SetAuthoringGraph(graph); }

		/// 根据文档变更摘要与脏区，对全部投影执行 Rebuild 或 ApplyDirtyRegion（内部策略唯一入口）。
		void RunProjectionPass(
			bool firstFrame,
			bool hasDocSignals,
			const SequenceDirtyRegion& dirty,
			const SequenceProjectionContext& ctx);

	private:
		void RebuildAll(const SequenceProjectionContext& ctx);
		void ApplyDirtyAll(const SequenceProjectionContext& ctx, const SequenceDirtyRegion& dirty);

		SequenceListProjection m_listProjection;
		SequenceTimelineProjection m_timelineProjection;
		SequenceGraphProjection m_graphProjection;
	};
}
