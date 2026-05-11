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
	class SequenceValidationRegistry;
	struct SequenceRuntimeOverlayState;
	class SequenceSearchViewModel;

	/// Read model for list / timeline / outliner; rebuilt from document + registry.
	/// 列表 / 时间轴 / 大纲的只读模型；由文档与注册表重建。
	class SequenceDocumentViewModel
	{
	public:
		void Rebuild(SequenceDocument& document, const SequenceComponentRegistry& registry);

		const std::vector<SequenceEntryViewModel>& GetVisibleEntries() const { return m_visibleRows; }

		void ApplySearchFilter(const std::string& filter);
		void ApplySearchViewModel(const SequenceSearchViewModel& search);

		void ApplyValidation(const SequenceValidationRegistry& registry, const SequenceDocument& document);
		void ApplyRuntimeOverlay(const SequenceRuntimeOverlayState& overlay);

		const std::vector<SequenceValidationIssue>& GetValidationIssues() const { return m_validationIssues; }

	private:
		void SyncVisibleWithStorage();

		std::vector<SequenceEntryViewModel> m_storage;
		std::vector<SequenceEntryViewModel> m_visibleRows;
		std::vector<SequenceValidationIssue> m_validationIssues;
	};
}
