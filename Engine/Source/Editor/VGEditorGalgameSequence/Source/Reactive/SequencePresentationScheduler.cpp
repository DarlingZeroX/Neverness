/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Reactive/SequencePresentationScheduler.h"

#include "AssetMonitoring/SequenceDependencyGraph.h"
#include "AuthoringGraph/SequenceAuthoringGraph.h"
#include "ComponentRegistry/SequenceComponentRegistry.h"
#include "DirtyRegions/SequenceDirtyRegion.h"
#include "DirtyRegions/SequenceDirtyRegionFlags.h"
#include "Document/SequenceDocument.h"
#include "Runtime/SequenceRuntimeObserver.h"
#include "Services/SequenceSearchIndexService.h"
#include "Services/SequenceValidationCacheService.h"
#include "Validation/SequenceValidationRegistry.h"
#include "ViewModels/SequenceDocumentViewModel.h"
#include "ViewModels/SequenceSearchViewModel.h"

#include <chrono>

namespace VisionGal::Editor
{
	namespace
	{
		void RunProjectionPass(
			const bool firstFrame,
			const bool hasDocSignals,
			const SequenceDirtyRegion& dirty,
			SequenceDocument& document,
			SequenceListProjection& list,
			SequenceTimelineProjection& timeline,
			SequenceGraphProjection& graph,
			const SequenceComponentRegistry& registry)
		{
			if (!firstFrame && !hasDocSignals)
				return;

			if (firstFrame)
			{
				list.Rebuild(document, registry);
				timeline.Rebuild(document, registry);
				graph.Rebuild(document, registry);
				return;
			}

			const bool structural =
				(dirty.Flags & SequenceDirtyRegionFlags::Structure) != SequenceDirtyRegionFlags::None;
			const bool property =
				(dirty.Flags & SequenceDirtyRegionFlags::Property) != SequenceDirtyRegionFlags::None;

			if (structural)
			{
				list.Rebuild(document, registry);
				timeline.Rebuild(document, registry);
				graph.Rebuild(document, registry);
			}
			else if (property && !dirty.Entries.empty())
			{
				list.ApplyDirtyRegion(dirty, document, registry);
				timeline.ApplyDirtyRegion(dirty, document, registry);
				graph.ApplyDirtyRegion(dirty, document, registry);
			}
			else
			{
				list.ApplyDirtyRegion(dirty, document, registry);
				timeline.ApplyDirtyRegion(dirty, document, registry);
				graph.ApplyDirtyRegion(dirty, document, registry);
			}
		}
	}

	void SequencePresentationScheduler::SetAuthoringGraph(SequenceAuthoringGraph* graph)
	{
		m_graphProjection.SetAuthoringGraph(graph);
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
		SequenceDependencyGraph& dependencyGraph,
		SequenceEditorEventBus* eventBus)
	{
		viewModel.SetListProjection(&m_listProjection);

		const bool firstFrame = !inOutFirstPresentationDone;
		inOutFirstPresentationDone = true;

		const bool hasDocSignals = firstFrame || mutSummary.StructuralChange || !mutSummary.TouchedIndices.empty()
			|| (dirty.Flags != SequenceDirtyRegionFlags::None);

		++m_metrics.PresentationTickCount;
		{
			const auto t0 = std::chrono::steady_clock::now();
			RunProjectionPass(
				firstFrame,
				hasDocSignals,
				dirty,
				document,
				m_listProjection,
				m_timelineProjection,
				m_graphProjection,
				componentRegistry);
			m_metrics.LastProjectionPassMicros = static_cast<uint64_t>(
				std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - t0).count());
		}

		if (firstFrame || hasDocSignals)
		{
			const auto t0 = std::chrono::steady_clock::now();
			dependencyGraph.RebuildFromDocument(document);
			searchIndex.RebuildFromViewStorageOrIncremental(document, viewModel.GetEntryStorage(), mutSummary, dirty);
			m_metrics.LastSearchRebuildMicros = static_cast<uint64_t>(
				std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - t0).count());
		}

		bool validationRefreshed = false;
		{
			const auto t0 = std::chrono::steady_clock::now();
			m_derivedStateGraph.InvalidateForPresentationTick(
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
			m_derivedStateGraph.Flush(dctx);
			m_metrics.LastDerivedPassMicros = static_cast<uint64_t>(
				std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - t0).count());
		}

		return validationRefreshed;
	}
}
