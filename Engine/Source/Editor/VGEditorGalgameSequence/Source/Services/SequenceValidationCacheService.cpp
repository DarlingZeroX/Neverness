/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Services/SequenceValidationCacheService.h"

#include "Document/SequenceDocument.h"
#include "Validation/SequenceValidationRegistry.h"

#include <algorithm>
#include <unordered_set>

namespace VisionGal::Editor
{
	void SequenceValidationCacheService::InvalidateAll()
	{
		m_stale = true;
		m_hasPendingSummary = false;
		m_pendingSummary = SequenceDocumentMutationSummary{};
		m_pendingSummary.StructuralChange = true;
	}

	void SequenceValidationCacheService::NotifyDocumentChanged(const SequenceDocumentMutationSummary& summary)
	{
		m_stale = true;
		if (!m_hasPendingSummary)
		{
			m_pendingSummary = summary;
			m_hasPendingSummary = true;
			return;
		}
		m_pendingSummary.StructuralChange = m_pendingSummary.StructuralChange || summary.StructuralChange;
		m_pendingSummary.TouchedIndices.insert(
			m_pendingSummary.TouchedIndices.end(), summary.TouchedIndices.begin(), summary.TouchedIndices.end());
	}

	void SequenceValidationCacheService::NotifyEntriesPropertyTouch(const std::vector<unsigned>& entryIndices)
	{
		SequenceDocumentMutationSummary s;
		s.StructuralChange = false;
		s.TouchedIndices = entryIndices;
		NotifyDocumentChanged(s);
	}

	bool SequenceValidationCacheService::ApplyIfStale(
		const SequenceDocument& document,
		const SequenceValidationRegistry& registry,
		uint64_t documentGenerationId)
	{
		if (!m_stale && documentGenerationId == m_lastAppliedGeneration)
			return false;

		SequenceDocumentMutationSummary summary{};
		if (m_hasPendingSummary)
			summary = m_pendingSummary;
		else
			summary.StructuralChange = true;

		const bool useFull = summary.StructuralChange || summary.TouchedIndices.empty();
		if (useFull)
		{
			m_issues = registry.RunAll(document);
		}
		else
		{
			auto touched = summary.TouchedIndices;
			std::sort(touched.begin(), touched.end());
			touched.erase(std::unique(touched.begin(), touched.end()), touched.end());

			std::unordered_set<unsigned> touchedSet(touched.begin(), touched.end());
			m_issues.erase(
				std::remove_if(
					m_issues.begin(),
					m_issues.end(),
					[&](const SequenceValidationIssue& issue) { return touchedSet.count(issue.EntryIndex) != 0; }),
				m_issues.end());
			const auto fresh = registry.RunForEntries(document, touched);
			m_issues.insert(m_issues.end(), fresh.begin(), fresh.end());
		}

		m_lastAppliedGeneration = documentGenerationId;
		m_stale = false;
		m_hasPendingSummary = false;
		m_pendingSummary = SequenceDocumentMutationSummary{};
		return true;
	}

	void SequenceValidationCacheService::ReplaceIssues(std::vector<SequenceValidationIssue> issues)
	{
		m_issues = std::move(issues);
		m_stale = false;
		m_hasPendingSummary = false;
	}
}
