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

#include "EditorSettingInterface.h"
#include "NNEditorFramework/Include/EditorCore/Localization.h"

namespace NN::Editor
{
	ImGuiTableFlags EditorSettingInterface::GetSettingTableFlag()
	{
		return ImGuiTableFlags_Hideable |
			ImGuiTableFlags_BordersOuter |
			ImGuiTableFlags_BordersV |
			ImGuiTableFlags_BordersInner |
			ImGuiTableFlags_Resizable;
	}

	void EditorSettingInterface::DrawTableColumnTitle(const std::string& title)
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text(EditorText{ title }.c_str());
		ImGui::TableNextColumn();
	}
}
