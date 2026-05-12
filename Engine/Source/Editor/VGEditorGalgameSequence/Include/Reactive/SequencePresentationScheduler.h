/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

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

	/// Centralizes presentation pipeline (Phase 6): projections → validation → overlay → search index.
	class SequencePresentationScheduler
	{
	public:
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
	};
}
