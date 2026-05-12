/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Reactive/DerivedState/SequenceDerivedStateGraph.h"

#include "DirtyRegions/SequenceDirtyRegion.h"
#include "DirtyRegions/SequenceDirtyRegionFlags.h"
#include "Document/SequenceDocument.h"
#include "Events/SequenceEditorEvent.h"
#include "Events/SequenceEditorEventBus.h"
#include "Runtime/SequenceRuntimeObserver.h"
#include "Services/SequenceSearchIndexService.h"
#include "Services/SequenceValidationCacheService.h"
#include "Validation/SequenceValidationRegistry.h"
#include "ViewModels/SequenceDocumentViewModel.h"
#include "ViewModels/SequenceSearchViewModel.h"

namespace VisionGal::Editor
{
	namespace
	{
		ReactiveCore::DerivedStateId ToCore(const SequenceDerivedStateId id)
		{
			return static_cast<ReactiveCore::DerivedStateId>(id);
		}
	}

	SequenceDerivedStateGraph::SequenceDerivedStateGraph() = default;

	void SequenceDerivedStateGraph::EnsureBuilt()
	{
		if (m_built)
			return;
		m_built = true;
		m_core.Clear();
		m_core.RegisterNode(
			ToCore(SequenceDerivedStateId::Validation),
			{},
			[this] {
				++m_validationComputeCount;
				if (m_activeCtx == nullptr || m_activeCtx->document == nullptr || m_activeCtx->viewModel == nullptr
					|| m_activeCtx->validationCache == nullptr || m_activeCtx->validationRegistry == nullptr)
					return;
				const bool refreshed = m_activeCtx->validationCache->ApplyIfStale(
					*m_activeCtx->document,
					*m_activeCtx->validationRegistry,
					m_activeCtx->document->GetGenerationId());
				m_activeCtx->viewModel->ApplyValidationIssues(m_activeCtx->validationCache->GetIssues());
				if (refreshed && m_activeCtx->eventBus != nullptr)
				{
					SequenceEditorEvent ev;
					ev.Type = SequenceEditorEventType::ValidationUpdated;
					m_activeCtx->eventBus->Publish(ev);
				}
				if (m_activeCtx->outValidationRefreshed != nullptr)
					*m_activeCtx->outValidationRefreshed = refreshed;
			});
		m_core.RegisterNode(
			ToCore(SequenceDerivedStateId::RuntimeOverlay),
			{},
			[this] {
				if (m_activeCtx == nullptr || m_activeCtx->viewModel == nullptr || m_activeCtx->runtimeObserver == nullptr)
					return;
				m_activeCtx->viewModel->ApplyRuntimeOverlay(m_activeCtx->runtimeObserver->GetOverlay());
			});
		m_core.RegisterNode(
			ToCore(SequenceDerivedStateId::SearchResults),
			{
				ToCore(SequenceDerivedStateId::Validation),
				ToCore(SequenceDerivedStateId::RuntimeOverlay),
			},
			[this] {
				++m_searchComputeCount;
				if (m_activeCtx == nullptr || m_activeCtx->viewModel == nullptr || m_activeCtx->searchIndex == nullptr
					|| m_activeCtx->searchViewModel == nullptr)
					return;
				m_activeCtx->viewModel->ApplySearchViewModelWithIndex(
					*m_activeCtx->searchIndex,
					*m_activeCtx->searchViewModel);
			});
	}

	void SequenceDerivedStateGraph::InvalidateForPresentationTick(
		const bool firstFrame,
		const bool hasDocSignals,
		const SequenceDocumentMutationSummary& mutSummary,
		const SequenceDirtyRegion& dirty,
		const uint64_t runtimeOverlayRevision)
	{
		EnsureBuilt();
		if (firstFrame || hasDocSignals)
		{
			m_core.Invalidate(ToCore(SequenceDerivedStateId::Validation));
			m_core.Invalidate(ToCore(SequenceDerivedStateId::RuntimeOverlay));
			m_core.Invalidate(ToCore(SequenceDerivedStateId::SearchResults));
			m_lastRuntimeRevision = runtimeOverlayRevision;
			return;
		}
		const bool structural =
			(dirty.Flags & SequenceDirtyRegionFlags::Structure) != SequenceDirtyRegionFlags::None;
		if (structural || mutSummary.StructuralChange)
		{
			m_core.Invalidate(ToCore(SequenceDerivedStateId::Validation));
			m_core.Invalidate(ToCore(SequenceDerivedStateId::SearchResults));
		}
		else if (!mutSummary.TouchedIndices.empty() || !dirty.Entries.empty()
			|| (dirty.Flags & SequenceDirtyRegionFlags::Property) != SequenceDirtyRegionFlags::None)
		{
			m_core.Invalidate(ToCore(SequenceDerivedStateId::Validation));
		}
		if (runtimeOverlayRevision != m_lastRuntimeRevision)
		{
			m_core.Invalidate(ToCore(SequenceDerivedStateId::RuntimeOverlay));
			m_lastRuntimeRevision = runtimeOverlayRevision;
		}
	}

	void SequenceDerivedStateGraph::Flush(SequenceDerivedStateTickContext& ctx)
	{
		EnsureBuilt();
		if (ctx.outValidationRefreshed != nullptr)
			*ctx.outValidationRefreshed = false;
		m_activeCtx = &ctx;
		m_core.FlushDirty();
		m_activeCtx = nullptr;
	}
}
