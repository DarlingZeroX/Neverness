/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#pragma once

#include <string>

namespace VisionGal::Editor
{
	class SequenceDependencyGraph;
	class SequenceDocument;
	class SequenceDocumentViewModel;
	class SequenceEditorEventBus;
	class SequenceRuntimeObserver;
	class SequenceSearchIndexService;
	class SequenceSearchViewModel;
	class SequenceValidationCacheService;
	class SequenceValidationRegistry;
	class SequenceDerivedStateGraph;
	struct SequenceDirtyRegion;
	struct SequenceDocumentMutationSummary;
	struct SequenceEditorMetrics;

	/// Phase 9：文档变更后的「依赖图 → 搜索索引 → 校验/Overlay/搜索派生」单一有序管线。
	/// 与 `SequenceProjectionPipeline` 解耦：先完成投影再调用本类，避免循环依赖。
	/// `RebuildFromDocument` 在 Tick 与资源变更路径均可能触发；依赖图对同代次文档重复重建为幂等替换。
	class SequenceDataConsistencyPipeline
	{
	public:
		/// 在投影 Pass 之后执行：按需重建依赖图与搜索索引，并 flush `SequenceDerivedStateGraph`。
		/// @return 是否在本帧刷新了校验派生（与 Phase 7 `Tick` 返回值语义一致）。
		[[nodiscard]] bool RunAfterProjections(
			bool firstFrame,
			bool hasDocSignals,
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
			SequenceEditorMetrics& metrics);

		/// 资源保存/外部变更：重建依赖图并按引用路径精确置脏校验缓存，最后请求 presentation 刷新。
		static void InvalidateReferencingEntriesForAsset(
			SequenceDocument& document,
			SequenceDependencyGraph& graph,
			const std::string& assetPath,
			SequenceValidationCacheService& validationCache,
			void (*requestPresentationRefresh)(void*),
			void* requestPresentationUserData);
	};
}
