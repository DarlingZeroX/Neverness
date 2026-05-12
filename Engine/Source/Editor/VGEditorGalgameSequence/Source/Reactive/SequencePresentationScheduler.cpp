/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Reactive/SequencePresentationScheduler.h"

#include "AssetMonitoring/SequenceDependencyGraph.h"
#include "ComponentRegistry/SequenceComponentRegistry.h"
#include "DirtyRegions/SequenceDirtyRegion.h"
#include "DirtyRegions/SequenceDirtyRegionFlags.h"
#include "Document/SequenceDocument.h"
#include "Events/SequenceEditorEvent.h"
#include "Events/SequenceEditorEventBus.h"
#include "Projection/SequenceGraphProjection.h"
#include "Projection/SequenceListProjection.h"
#include "Projection/SequenceTimelineProjection.h"
#include "Runtime/SequenceRuntimeObserver.h"
#include "Services/SequenceSearchIndexService.h"
#include "Services/SequenceValidationCacheService.h"
#include "Validation/SequenceValidationRegistry.h"
#include "ViewModels/SequenceDocumentViewModel.h"
#include "ViewModels/SequenceSearchViewModel.h"

namespace VisionGal::Editor
{
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
		SequenceDependencyGraph& dependencyGraph,
		SequenceEditorEventBus* eventBus)
	{
		static SequenceListProjection s_list;
		static SequenceTimelineProjection s_timeline;
		static SequenceGraphProjection s_graph;

		const bool firstFrame = !inOutFirstPresentationDone;
		inOutFirstPresentationDone = true;

		const bool hasDocSignals = firstFrame || mutSummary.StructuralChange || !mutSummary.TouchedIndices.empty()
			|| (dirty.Flags != SequenceDirtyRegionFlags::None);

		if (firstFrame || hasDocSignals)
		{
			s_list.Apply(firstFrame, dirty, document, viewModel, componentRegistry);
			s_timeline.Apply(firstFrame, dirty, document, viewModel, componentRegistry);
			s_graph.Apply(firstFrame, dirty, document, viewModel, componentRegistry);
			dependencyGraph.RebuildFromDocument(document);
			searchIndex.RebuildFromViewStorage(viewModel.GetEntryStorage());
		}

		const bool validationRefreshed = validationCache.ApplyIfStale(
			document, validationRegistry, document.GetGenerationId());
		viewModel.ApplyValidationIssues(validationCache.GetIssues());

		if (validationRefreshed && eventBus != nullptr)
		{
			SequenceEditorEvent ev;
			ev.Type = SequenceEditorEventType::ValidationUpdated;
			eventBus->Publish(ev);
		}

		viewModel.ApplyRuntimeOverlay(runtimeObserver.GetOverlay());
		viewModel.ApplySearchViewModelWithIndex(searchIndex, searchViewModel);

		return validationRefreshed;
	}
}
