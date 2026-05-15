/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "Projection/ISequenceProjection.h"
#include "Validation/SequenceValidationIssue.h"
#include "ViewModels/SequenceEntryViewModel.h"

#include <vector>

namespace VisionGal::Editor
{
	struct SequenceRuntimeOverlayState;

	/// List read-model: owns entry rows; scheduler applies dirty / derived overlays.
	class SequenceListProjection final : public ISequenceProjection
	{
	public:
		void Rebuild(const SequenceProjectionContext& ctx) override;

		void ApplyDirtyRegion(const SequenceDirtyRegion& dirty, const SequenceProjectionContext& ctx) override;

		[[nodiscard]] const std::vector<SequenceEntryViewModel>& GetEntryRows() const { return m_entryRows; }

		void ApplyValidationIssues(const std::vector<SequenceValidationIssue>& issues);
		void ApplyRuntimeOverlay(const SequenceRuntimeOverlayState& overlay);

	private:
		std::vector<SequenceEntryViewModel> m_entryRows;
	};
}
