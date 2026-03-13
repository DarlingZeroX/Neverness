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
#include <HCore/Interface/HConfig.h>
#include <VGImgui/IncludeImGui.h>
#include <VGImgui/IncludeImGuiEx.h>

namespace Horizon::Editor {
	struct PanelInterface : ImGuiEx::ImPanelInterface
	{
		~PanelInterface() override = default;
	};
}
