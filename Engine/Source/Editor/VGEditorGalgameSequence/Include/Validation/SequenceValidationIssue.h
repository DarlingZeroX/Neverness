/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include <cstdint>
#include <string>

namespace VisionGal::Editor
{
	enum class SequenceValidationSeverity : uint8_t
	{
		Info = 0,
		Warning = 1,
		Error = 2,
	};

	struct SequenceValidationIssue
	{
		uint32_t EntryIndex = 0;
		SequenceValidationSeverity Severity = SequenceValidationSeverity::Warning;
		std::string Message;
		std::string RuleId;
	};
}
