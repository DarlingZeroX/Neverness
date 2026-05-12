/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "Properties/SequencePropertyDescriptor.h"

#include <string>
#include <vector>

namespace VisionGal::Editor
{
	/// Data-driven description of a sequence component type for palette / menus / future plugins.
	/// 序列组件类型的数据驱动描述，用于调色板、菜单及未来插件等。
	struct SequenceComponentMetadata
	{
		std::string TypeNameID;
		std::string DisplayName;
		std::string Description;
		/// Stable palette grouping key (displayed to user; may match localized category name).
		/// 稳定的调色板分组键（对用户显示；可与本地化分类名一致）。
		std::string Category;
		/// FontAwesome UTF-8 glyph(s)
		/// FontAwesome 图标的 UTF-8 字形（一个或多个）。
		std::string Icon;
		int Priority = 0;

		std::vector<SequencePropertyDescriptor> PropertyDescriptors;

		/// Human-readable name for lists / inspector title fallback.
		[[nodiscard]] std::string PrimaryLabel() const
		{
			return DisplayName.empty() ? TypeNameID : DisplayName;
		}

		/// Palette button / grid cell text (icon + primary, trimmed of redundant spaces).
		[[nodiscard]] std::string PaletteButtonText() const
		{
			const std::string primary = PrimaryLabel();
			if (Icon.empty())
				return primary;
			if (primary.empty())
				return Icon;
			return Icon + " " + primary;
		}
	};
}
