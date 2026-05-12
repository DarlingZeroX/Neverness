/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "Validation/ISequenceValidator.h"

namespace VisionGal::Editor
{
	class SequenceComponentRegistry;

	class EmptyDialogueValidator final : public ISequenceValidator
	{
	public:
		explicit EmptyDialogueValidator(const SequenceComponentRegistry* componentRegistry = nullptr);

		[[nodiscard]] std::vector<SequenceValidationIssue> Validate(const SequenceDocument& document) const override;
		[[nodiscard]] std::vector<SequenceValidationIssue> ValidateEntries(
			const SequenceDocument& document,
			const std::vector<unsigned>& entryIndices) const override;
		[[nodiscard]] const char* GetRuleId() const override { return "Builtin.EmptyDialogue"; }

	private:
		const SequenceComponentRegistry* m_componentRegistry = nullptr;
	};
}
