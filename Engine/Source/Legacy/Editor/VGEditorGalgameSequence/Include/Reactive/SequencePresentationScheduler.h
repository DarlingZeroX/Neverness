/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "Reactive/SequenceEditorMetrics.h"

#include "Projection/SequenceProjectionPipeline.h"
#include "Reactive/DerivedState/SequenceDerivedStateGraph.h"
#include "Services/SequenceDataConsistencyPipeline.h"

namespace VisionGal::Editor
{
	class SequenceAuthoringGraph;
	class SequenceComponentRegistry;
	class SequenceDependencyGraph;
	class SequenceDocument;
	class SequenceDocumentViewModel;
	class SequenceEditorEventBus;
	class SequenceRuntimeObserver;
	class SequenceSearchIndexService;
	class SequenceSearchViewModel;
	class SequenceSelectionModel;
	class SequenceValidationCacheService;
	class SequenceValidationRegistry;
	struct SequenceDirtyRegion;
	struct SequenceDocumentMutationSummary;

	/// Presentation pipeline (Phase 7/9)：派生 Pass → **SequenceProjectionPipeline** → 派生读模型与服务。
	class SequencePresentationScheduler
	{
	public:
		[[nodiscard]] SequenceListProjection& GetListProjection() { return m_projectionPipeline.GetListProjection(); }
		[[nodiscard]] const SequenceListProjection& GetListProjection() const
		{
			return m_projectionPipeline.GetListProjection();
		}
		[[nodiscard]] SequenceGraphProjection& GetGraphProjection() { return m_projectionPipeline.GetGraphProjection(); }
		[[nodiscard]] SequenceTimelineProjection& GetTimelineProjection()
		{
			return m_projectionPipeline.GetTimelineProjection();
		}

		[[nodiscard]] SequenceProjectionPipeline& GetProjectionPipeline() { return m_projectionPipeline; }
		[[nodiscard]] const SequenceProjectionPipeline& GetProjectionPipeline() const { return m_projectionPipeline; }
		[[nodiscard]] SequenceDataConsistencyPipeline& GetDataConsistencyPipeline() { return m_dataConsistencyPipeline; }
		[[nodiscard]] const SequenceDataConsistencyPipeline& GetDataConsistencyPipeline() const
		{
			return m_dataConsistencyPipeline;
		}

		[[nodiscard]] const SequenceEditorMetrics& GetLastMetrics() const { return m_metrics; }

		void SetAuthoringGraph(SequenceAuthoringGraph* graph);

		[[nodiscard]] bool Tick(
			bool& inOutFirstPresentationDone,
			const SequenceDocumentMutationSummary& mutSummary,
			const SequenceDirtyRegion& dirty,
			SequenceDocument& document,
			SequenceDocumentViewModel& viewModel,
			SequenceComponentRegistry& componentRegistry,
			SequenceValidationCacheService& validationCache,
			SequenceValidationRegistry& validationRegistry,
			SequenceSearchIndexService& searchIndex,
			SequenceRuntimeObserver& runtimeObserver,
			SequenceSearchViewModel& searchViewModel,
			SequenceSelectionModel& selectionModel,
			SequenceDependencyGraph& dependencyGraph,
			SequenceEditorEventBus* eventBus);

	private:
		SequenceProjectionPipeline m_projectionPipeline;
		SequenceDataConsistencyPipeline m_dataConsistencyPipeline;
		SequenceDerivedStateGraph m_derivedStateGraph;
		SequenceEditorMetrics m_metrics{};
	};
}
