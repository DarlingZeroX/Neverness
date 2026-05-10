/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include <string>

namespace VisionGal::Editor
{
	/// Data-driven description of a sequence component type for palette / menus / future plugins.
	struct SequenceComponentMetadata
	{
		std::string TypeNameID;
		std::string DisplayName;
		std::string Description;
		/// Stable palette grouping key (displayed to user; may match localized category name).
		std::string Category;
		/// FontAwesome UTF-8 glyph(s)
		std::string Icon;
		int Priority = 0;
	};
}
