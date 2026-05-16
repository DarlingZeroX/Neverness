/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * For the latest information, see https://darlingzerox.github.io/VisionGalDoc/
 * GitHub page: https://github.com/DarlingZeroX/VisionGal
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#pragma once
#include "../../Config.h"

namespace VisionGal::Editor
{
	struct VG_EDITOR_FRAMEWORK_API EditorStyle
	{
		static void DarkTheme();

		static const std::vector<std::string>& GetEditorThemes();

		static bool SetTheme(const std::string& name);
	};
}
