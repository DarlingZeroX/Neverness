/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "Reactive/DerivedState/SequenceDerivedStateId.h"

#include "ReactiveCore/DerivedStateGraph.h"

namespace VisionGal::Editor
{
	class SequenceEditorEventBus;
	class SequenceDocument;
	class SequenceDocumentViewModel;
	class SequenceRuntimeObserver;
	class SequenceSearchIndexService;
	class SequenceSearchViewModel;
	class SequenceValidationCacheService;
	class SequenceValidationRegistry;
	struct SequenceDirtyRegion;
	struct SequenceDocumentMutationSummary;

	/// 装配 `VGEditorReactive` 内核与序列校验 / Overlay / 搜索派生 Pass。
	struct SequenceDerivedStateTickContext
	{
		SequenceDocument* document = nullptr;
		SequenceDocumentViewModel* viewModel = nullptr;
		SequenceValidationCacheService* validationCache = nullptr;
		SequenceValidationRegistry* validationRegistry = nullptr;
		SequenceSearchIndexService* searchIndex = nullptr;
		SequenceRuntimeObserver* runtimeObserver = nullptr;
		SequenceSearchViewModel* searchViewModel = nullptr;
		SequenceEditorEventBus* eventBus = nullptr;
		bool* outValidationRefreshed = nullptr;
	};

	class SequenceDerivedStateGraph
	{
	public:
		SequenceDerivedStateGraph();
		void EnsureBuilt();
		void InvalidateForPresentationTick(
			bool firstFrame,
			bool hasDocSignals,
			const SequenceDocumentMutationSummary& mutSummary,
			const SequenceDirtyRegion& dirty,
			uint64_t runtimeOverlayRevision);
		void Flush(SequenceDerivedStateTickContext& ctx);
		[[nodiscard]] uint32_t GetValidationComputeCount() const { return m_validationComputeCount; }
		[[nodiscard]] uint32_t GetSearchComputeCount() const { return m_searchComputeCount; }

	private:
		ReactiveCore::DerivedStateGraph m_core;
		SequenceDerivedStateTickContext* m_activeCtx = nullptr;
		bool m_built = false;
		uint64_t m_lastRuntimeRevision = 0;
		uint32_t m_validationComputeCount = 0;
		uint32_t m_searchComputeCount = 0;
	};
}
