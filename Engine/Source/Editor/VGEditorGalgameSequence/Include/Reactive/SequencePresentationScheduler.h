/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "Reactive/SequenceEditorMetrics.h"

#include "Projection/SequenceGraphProjection.h"
#include "Projection/SequenceListProjection.h"
#include "Projection/SequenceTimelineProjection.h"

namespace VisionGal::Editor
{
	class SequenceComponentRegistry;
	class SequenceDependencyGraph;
	class SequenceDocument;
	class SequenceDocumentViewModel;
	class SequenceEditorEventBus;
	class SequenceRuntimeObserver;
	class SequenceSearchIndexService;
	class SequenceSearchViewModel;
	class SequenceValidationCacheService;
	class SequenceValidationRegistry;
	struct SequenceDirtyRegion;
	struct SequenceDocumentMutationSummary;

	/// Presentation pipeline (Phase 7): reactive pass → projections → derived read-models → services.
	class SequencePresentationScheduler
	{
	public:
		[[nodiscard]] SequenceListProjection& GetListProjection() { return m_listProjection; }
		[[nodiscard]] const SequenceListProjection& GetListProjection() const { return m_listProjection; }
		[[nodiscard]] SequenceGraphProjection& GetGraphProjection() { return m_graphProjection; }
		[[nodiscard]] SequenceTimelineProjection& GetTimelineProjection() { return m_timelineProjection; }

		[[nodiscard]] const SequenceEditorMetrics& GetLastMetrics() const { return m_metrics; }

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
			SequenceDependencyGraph& dependencyGraph,
			SequenceEditorEventBus* eventBus);

	private:
		SequenceListProjection m_listProjection;
		SequenceTimelineProjection m_timelineProjection;
		SequenceGraphProjection m_graphProjection;
		SequenceEditorMetrics m_metrics{};
	};
}
