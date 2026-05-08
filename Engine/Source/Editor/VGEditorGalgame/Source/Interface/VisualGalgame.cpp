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

#include "VisualGalgame.h"
#include <VGImgui/IncludeImGui.h>

namespace VisionGal::Editor
{
	VisualGalEditor::VisualGalEditor()
	{
	}

	VisualGalEditor::~VisualGalEditor()
	{
	}

	void VisualGalEditor::Initialize()
	{
	}

	void VisualGalEditor::DrawEditorWindow()
	{
		if (ImGui::Begin("Visual GalGame Editor",nullptr ,ImGuiWindowFlags_MenuBar))
		{
			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu("File"))
				{
					if (ImGui::MenuItem("Save"))
					{
						m_ScriptSequence.SaveTest();
					}
					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}

			m_ScriptSequence.RenderSequenceUI();
		}

		ImGui::End();
	}
}
