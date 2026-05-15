/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "ViewModels/SequenceEntryViewModel.h"

#include <string>
#include <vector>

namespace VisionGal::Editor
{
	class SequenceDocument;
	class SequenceComponentRegistry;
	struct SequenceDocumentMutationSummary;
	struct SequenceDirtyRegion;

	/// Normalized per-entry text for substring search (editor thread).
	class SequenceSearchIndexService
	{
	public:
		void RebuildFromViewStorage(const std::vector<SequenceEntryViewModel>& rows);

		/// 大文档：小范围变更时按行更新索引；结构变更或大范围仍全量重建。
		void RebuildFromViewStorageOrIncremental(
			const SequenceDocument& document,
			const std::vector<SequenceEntryViewModel>& rows,
			const SequenceDocumentMutationSummary& mutSummary,
			const SequenceDirtyRegion& dirty);

		/// Returns entry indices whose indexed text contains `needle` (case-insensitive). Empty needle matches all.
		[[nodiscard]] std::vector<unsigned> QueryTextIndices(const std::string& needle) const;

	private:
		std::vector<std::string> m_normalizedLines;
	};
}
