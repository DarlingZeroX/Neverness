/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Reactive/SequencePresentationScheduler.h"

#include "Core/SequenceSelectionModel.h"
#include "DirtyRegions/SequenceDirtyRegion.h"
#include "DirtyRegions/SequenceDirtyRegionFlags.h"
#include "Document/SequenceDocument.h"
#include "Projection/SequenceProjectionContext.h"
#include "Runtime/SequenceRuntimeObserver.h"
#include "Services/SequenceSearchIndexService.h"
#include "Services/SequenceValidationCacheService.h"
#include "Validation/SequenceValidationRegistry.h"
#include "ViewModels/SequenceDocumentViewModel.h"
#include "ViewModels/SequenceSearchViewModel.h"

#include <chrono>

namespace VisionGal::Editor
{
	void SequencePresentationScheduler::SetAuthoringGraph(SequenceAuthoringGraph* graph)
	{
		m_projectionPipeline.SetAuthoringGraph(graph);
	}

	bool SequencePresentationScheduler::Tick(
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
		SequenceEditorEventBus* eventBus)
	{
		viewModel.SetListProjection(&m_projectionPipeline.GetListProjection());

		const bool firstFrame = !inOutFirstPresentationDone;
		inOutFirstPresentationDone = true;

		const bool hasDocSignals = firstFrame || mutSummary.StructuralChange || !mutSummary.TouchedIndices.empty()
			|| (dirty.Flags != SequenceDirtyRegionFlags::None);

		SequenceProjectionContext projCtx;
		projCtx.document = &document;
		projCtx.validation = &validationCache;
		projCtx.runtime = &runtimeObserver.GetOverlay();
		projCtx.search = &searchIndex;
		projCtx.selection = &selectionModel;
		projCtx.registry = &componentRegistry;

		++m_metrics.PresentationTickCount;
		{
			const auto t0 = std::chrono::steady_clock::now();
			m_projectionPipeline.RunProjectionPass(firstFrame, hasDocSignals, dirty, projCtx);
			m_metrics.LastProjectionPassMicros = static_cast<uint64_t>(
				std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - t0).count());
		}

		return m_dataConsistencyPipeline.RunAfterProjections(
			firstFrame,
			hasDocSignals,
			mutSummary,
			dirty,
			document,
			viewModel,
			validationCache,
			validationRegistry,
			searchIndex,
			runtimeObserver,
			searchViewModel,
			dependencyGraph,
			m_derivedStateGraph,
			eventBus,
			m_metrics);
	}
}
