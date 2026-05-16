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
#include <vector>

namespace VisionGal::Editor
{
	/// Enum 型属性的展示项；`Value` 为写入组件的整型约定（首版与 ImGui 下拉索引一致即可）。
	struct SequenceEnumItem
	{
		std::int64_t Value = 0;
		std::string DisplayLabel;
	};

	/// 附属于 `SequencePropertyType::Enum` 的条目表。
	struct SequenceEnumSchema
	{
		std::vector<SequenceEnumItem> Items;
	};
}
