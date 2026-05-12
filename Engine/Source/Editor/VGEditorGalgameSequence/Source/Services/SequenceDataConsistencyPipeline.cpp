/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Services/SequenceDataConsistencyPipeline.h"

#include "AssetMonitoring/SequenceDependencyGraph.h"
#include "DirtyRegions/SequenceDirtyRegion.h"
#include "Document/SequenceDocument.h"
#include "Events/SequenceEditorEvent.h"
#include "Reactive/DerivedState/SequenceDerivedStateGraph.h"
#include "Reactive/SequenceEditorMetrics.h"
#include "Runtime/SequenceRuntimeObserver.h"
#include "Services/SequenceSearchIndexService.h"
#include "Services/SequenceValidationCacheService.h"
#include "Validation/SequenceValidationRegistry.h"
#include "ViewModels/SequenceDocumentViewModel.h"
#include "ViewModels/SequenceSearchViewModel.h"

#include <chrono>
#include <vector>

namespace VisionGal::Editor
{
	bool SequenceDataConsistencyPipeline::RunAfterProjections(
		const bool firstFrame,
		const bool hasDocSignals,
		const SequenceDocumentMutationSummary& mutSummary,
		const SequenceDirtyRegion& dirty,
		SequenceDocument& document,
		SequenceDocumentViewModel& viewModel,
		SequenceValidationCacheService& validationCache,
		SequenceValidationRegistry& validationRegistry,
		SequenceSearchIndexService& searchIndex,
		SequenceRuntimeObserver& runtimeObserver,
		SequenceSearchViewModel& searchViewModel,
		SequenceDependencyGraph& dependencyGraph,
		SequenceDerivedStateGraph& derivedStateGraph,
		SequenceEditorEventBus* eventBus,
		SequenceEditorMetrics& metrics)
	{
		if (firstFrame || hasDocSignals)
		{
			const auto t0 = std::chrono::steady_clock::now();
			dependencyGraph.RebuildFromDocument(document);
			searchIndex.RebuildFromViewStorageOrIncremental(document, viewModel.GetEntryStorage(), mutSummary, dirty);
			metrics.LastSearchRebuildMicros = static_cast<uint64_t>(
				std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - t0).count());
		}

		bool validationRefreshed = false;
		{
			const auto t0 = std::chrono::steady_clock::now();
			derivedStateGraph.InvalidateForPresentationTick(
				firstFrame,
				hasDocSignals,
				mutSummary,
				dirty,
				runtimeObserver.GetOverlayRevision());
			SequenceDerivedStateTickContext dctx;
			dctx.document = &document;
			dctx.viewModel = &viewModel;
			dctx.validationCache = &validationCache;
			dctx.validationRegistry = &validationRegistry;
			dctx.searchIndex = &searchIndex;
			dctx.runtimeObserver = &runtimeObserver;
			dctx.searchViewModel = &searchViewModel;
			dctx.eventBus = eventBus;
			dctx.outValidationRefreshed = &validationRefreshed;
			derivedStateGraph.Flush(dctx);
			metrics.LastDerivedPassMicros = static_cast<uint64_t>(
				std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - t0).count());
		}

		return validationRefreshed;
	}

	void SequenceDataConsistencyPipeline::InvalidateReferencingEntriesForAsset(
		SequenceDocument& document,
		SequenceDependencyGraph& graph,
		const std::string& assetPath,
		SequenceValidationCacheService& validationCache,
		void (*requestPresentationRefresh)(void*),
		void* requestPresentationUserData)
	{
		graph.RebuildFromDocument(document);
		const std::vector<unsigned> touched = graph.EntriesReferencing(assetPath);
		if (touched.empty())
			return;
		validationCache.NotifyEntriesPropertyTouch(touched);
		if (requestPresentationRefresh != nullptr)
			requestPresentationRefresh(requestPresentationUserData);
	}
}
