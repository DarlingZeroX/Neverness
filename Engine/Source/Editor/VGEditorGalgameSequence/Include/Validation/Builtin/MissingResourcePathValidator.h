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
	class MissingResourcePathValidator final : public ISequenceValidator
	{
	public:
		[[nodiscard]] std::vector<SequenceValidationIssue> Validate(const SequenceDocument& document) const override;
		[[nodiscard]] const char* GetRuleId() const override { return "Builtin.MissingResourcePath"; }
	};
}
