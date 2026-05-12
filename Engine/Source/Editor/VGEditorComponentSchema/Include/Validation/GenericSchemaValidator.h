/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "Schema/SequencePropertySchema.h"

#include <string>
#include <vector>

namespace VisionGal::Editor
{
	/// 单条 Schema 规则违反说明（由宿主映射为 `SequenceValidationIssue` 等）。
	struct SequenceSchemaValidationNote
	{
		std::string Message;
	};

	/// 基于 Flags / Range / 类型对当前取值做通用校验（不涉及 VFS / 跨条目语义）。
	[[nodiscard]] std::vector<SequenceSchemaValidationNote> ValidatePropertyValue(
		const SequencePropertySchema& prop,
		const SequencePropertyValue& current);
}
