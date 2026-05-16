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

#include "EditorCore/MenuItem.h"
#include <NNRuntimeImGui/IncludeImGui.h>

namespace NN::Editor
{
	void EditorUIMenu::OnGUI()
	{
		for (auto& item : m_MenuItem) {
			if (ImGui::MenuItem(item.label.c_str(), item.shortcut.c_str(), item.p_selected, item.enabled)) {
				if (item.callback)
				{
					item.callback();
				}
			}
		}
	}

	void EditorUIMenu::AddMenuItem(const MenuItem& item)
	{
		m_MenuItem.push_back(item);
	}
}


