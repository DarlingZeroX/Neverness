/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "Validation/SequenceValidationIssue.h"
#include "ViewModels/SequenceEntryViewModel.h"

#include <vector>

namespace VisionGal::Editor
{
	class SequenceComponentRegistry;
	class SequenceDocument;
	class SequenceListProjection;
	class SequenceValidationRegistry;
	struct SequenceRuntimeOverlayState;
	class SequenceSearchViewModel;
	class SequenceSearchIndexService;

	/// Facade over list projection read-model + visible/filter + validation list.
	class SequenceDocumentViewModel
	{
	public:
		void SetListProjection(SequenceListProjection* listProjection) { m_listProjection = listProjection; }

		void Rebuild(SequenceDocument& document, const SequenceComponentRegistry& registry);
		void RebuildEntriesAtIndices(
			SequenceDocument& document,
			const SequenceComponentRegistry& registry,
			const std::vector<unsigned>& indices);

		[[nodiscard]] const std::vector<SequenceEntryViewModel>& GetEntryStorage() const;

		const std::vector<SequenceEntryViewModel>& GetVisibleEntries() const { return m_visibleRows; }

		void ApplySearchFilter(const std::string& filter);
		void ApplySearchViewModel(const SequenceSearchViewModel& search);
		void ApplySearchViewModelWithIndex(const SequenceSearchIndexService& index, const SequenceSearchViewModel& search);

		void ApplyValidationIssues(const std::vector<SequenceValidationIssue>& issues);
		void ApplyValidation(const SequenceValidationRegistry& registry, const SequenceDocument& document);
		void ApplyRuntimeOverlay(const SequenceRuntimeOverlayState& overlay);

		const std::vector<SequenceValidationIssue>& GetValidationIssues() const { return m_validationIssues; }

	private:
		void SyncVisibleWithStorage();

		SequenceListProjection* m_listProjection = nullptr;
		std::vector<SequenceEntryViewModel> m_storage;
		std::vector<SequenceEntryViewModel> m_visibleRows;
		std::vector<SequenceValidationIssue> m_validationIssues;
	};
}
