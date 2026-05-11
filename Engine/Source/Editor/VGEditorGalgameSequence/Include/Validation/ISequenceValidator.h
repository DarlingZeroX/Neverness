/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "Validation/SequenceValidationIssue.h"

#include <vector>

namespace VisionGal::Editor
{
	class SequenceDocument;

	/// Pluggable row-level validation (no ImGui).
	/// 可插拔的行级校验（无 ImGui）。
	class ISequenceValidator
	{
	public:
		virtual ~ISequenceValidator() = default;

		[[nodiscard]] virtual std::vector<SequenceValidationIssue> Validate(const SequenceDocument& document) const = 0;
		[[nodiscard]] virtual const char* GetRuleId() const = 0;
	};
}
