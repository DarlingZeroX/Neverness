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
	/// One row in the sequence list / timeline / outliner (presentation only).
	/// 序列列表 / 时间轴 / 大纲中的一行（仅展示层）。
	struct SequenceEntryViewModel
	{
		/// Index in `VGSSequenceDataContainer::m_Sequence` (real command index).
		/// 在 `VGSSequenceDataContainer::m_Sequence` 中的索引（真实命令参数）。
		uint32_t EntryIndex = 0;
		std::string TypeNameID;
		std::string DisplayName;
		std::string Subtitle;
		std::string Category;
		std::string Icon;
		bool HasValidationError = false;
		bool RuntimeHighlight = false;
	};
}
