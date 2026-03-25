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
#include "../../VGEditorCoreConfig.h"
#include <string>
#include <HCore/Include/File/NlohmannJson.h>
#include "VGImgui/Include/Imgui/imgui.h"

namespace VisionGal::Editor
{
	struct EditorSettingInterface: public IPanel
	{
		~EditorSettingInterface() override = default;

		virtual void Load(const nlohmann::json& json) = 0;
		virtual void Save(nlohmann::json& json) = 0;

		static ImGuiTableFlags GetSettingTableFlag();
		static void DrawTableColumnTitle(const std::string& title);
	};


}
