/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "Schema/SequenceEnumSchema.h"
#include "Schema/SequencePropertyAccessor.h"
#include "Schema/SequencePropertyFlags.h"
#include "Schema/SequencePropertyRange.h"
#include "Schema/SequencePropertyType.h"
#include "Schema/SequencePropertyValue.h"

#include <optional>
#include <string>

namespace VisionGal::Editor
{
	/// 单一属性的完整 Authoring 描述（Inspector / 校验 / 搜索 / Patch 的共用源）。
	struct SequencePropertySchema
	{
		SequencePropertyType Type = SequencePropertyType::Unknown;
		/// 稳定机器名，用于 Patch 路径、搜索键与脚本导出。
		std::string Name;
		/// 面板展示名。
		std::string DisplayName;
		std::string Category;
		SequencePropertyFlags Flags = SequencePropertyFlags::None;
		std::optional<SequencePropertyRange> Range;
		std::optional<SequenceEnumSchema> Enum;
		SequencePropertyValue DefaultValue;
		SequencePropertyAccessor Accessor;
		bool Editable = true;
	};
}
