/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "Events/SequenceEditorEvent.h"
#include "Validation/SequenceValidationIssue.h"

#include <vector>

namespace VisionGal::Editor
{
	class SequenceDocument;
	class SequenceValidationRegistry;

	/// Aggregated validation with incremental refresh on non-structural edits.
	class SequenceValidationCacheService
	{
	public:
		void InvalidateAll();
		void NotifyDocumentChanged(const SequenceDocumentMutationSummary& summary);

		void NotifyEntriesPropertyTouch(const std::vector<unsigned>& entryIndices);

		/// Applies pending work when `m_stale` is set. No-op if document generation unchanged and not stale.
		[[nodiscard]] bool ApplyIfStale(
			const SequenceDocument& document,
			const SequenceValidationRegistry& registry,
			uint64_t documentGenerationId);

		void ReplaceIssues(std::vector<SequenceValidationIssue> issues);

		[[nodiscard]] const std::vector<SequenceValidationIssue>& GetIssues() const { return m_issues; }

	private:
		std::vector<SequenceValidationIssue> m_issues;
		SequenceDocumentMutationSummary m_pendingSummary{};
		bool m_hasPendingSummary = false;
		bool m_stale = true;
		uint64_t m_lastAppliedGeneration = 0;
	};
}
