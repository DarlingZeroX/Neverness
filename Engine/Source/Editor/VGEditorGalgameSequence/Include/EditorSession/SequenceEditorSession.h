/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "AuthoringGraph/SequenceAuthoringGraph.h"
#include "Extensions/SequenceExtensionRegistry.h"
#include "Projection/ProjectionEvents/SequenceProjectionEventBus.h"
#include "RuntimeBridge/SequenceRuntimeEventTimeline.h"

namespace VisionGal::Editor
{
	/// 将 Authoring 图、投影总线、扩展注册表与运行时时间线从宿主中拆出（Phase 8）。
	class SequenceEditorSession
	{
	public:
		[[nodiscard]] SequenceAuthoringGraph& GetAuthoringGraph() { return m_authoringGraph; }
		[[nodiscard]] SequenceProjectionEventBus& GetProjectionEventBus() { return m_projectionEventBus; }
		[[nodiscard]] SequenceExtensionRegistry& GetExtensionRegistry() { return m_extensionRegistry; }
		[[nodiscard]] SequenceRuntimeEventTimeline& GetRuntimeEventTimeline() { return m_runtimeTimeline; }

	private:
		SequenceAuthoringGraph m_authoringGraph;
		SequenceProjectionEventBus m_projectionEventBus;
		SequenceExtensionRegistry m_extensionRegistry;
		SequenceRuntimeEventTimeline m_runtimeTimeline{ 8192 };
	};
}
