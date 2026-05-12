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
	class SequenceDataConsistencyPipeline;
	class SequenceMutationPipeline;
	class SequenceProjectionPipeline;
	class SequenceRuntimeKernel;

	/// Phase 8/9：将 Authoring 图、投影总线、扩展注册表与运行时时间线从宿主拆出；
	/// Phase 9 起可挂载各「管线」指针，便于扩展与单测探测同一宿主上的活跃子系统。
	class SequenceEditorSession
	{
	public:
		[[nodiscard]] SequenceAuthoringGraph& GetAuthoringGraph() { return m_authoringGraph; }
		[[nodiscard]] SequenceProjectionEventBus& GetProjectionEventBus() { return m_projectionEventBus; }
		[[nodiscard]] SequenceExtensionRegistry& GetExtensionRegistry() { return m_extensionRegistry; }
		[[nodiscard]] SequenceRuntimeEventTimeline& GetRuntimeEventTimeline() { return m_runtimeTimeline; }

		void SetProjectionPipeline(SequenceProjectionPipeline* p) { m_projectionPipeline = p; }
		void SetDataConsistencyPipeline(SequenceDataConsistencyPipeline* p) { m_dataConsistencyPipeline = p; }
		void SetMutationPipeline(SequenceMutationPipeline* p) { m_mutationPipeline = p; }
		void SetRuntimeKernel(SequenceRuntimeKernel* k) { m_runtimeKernel = k; }

		[[nodiscard]] SequenceProjectionPipeline* GetProjectionPipeline() const { return m_projectionPipeline; }
		[[nodiscard]] SequenceDataConsistencyPipeline* GetDataConsistencyPipeline() const
		{
			return m_dataConsistencyPipeline;
		}
		[[nodiscard]] SequenceMutationPipeline* GetMutationPipeline() const { return m_mutationPipeline; }
		[[nodiscard]] SequenceRuntimeKernel* GetRuntimeKernel() const { return m_runtimeKernel; }

	private:
		SequenceAuthoringGraph m_authoringGraph;
		SequenceProjectionEventBus m_projectionEventBus;
		SequenceExtensionRegistry m_extensionRegistry;
		SequenceRuntimeEventTimeline m_runtimeTimeline{ 8192 };

		SequenceProjectionPipeline* m_projectionPipeline = nullptr;
		SequenceDataConsistencyPipeline* m_dataConsistencyPipeline = nullptr;
		SequenceMutationPipeline* m_mutationPipeline = nullptr;
		SequenceRuntimeKernel* m_runtimeKernel = nullptr;
	};
}
