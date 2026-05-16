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

#include "Setting/PreferencesPanel.h"
#include <NNRuntimeImGui/IncludeImGuiEx.h>

#include "EditorCore/EditorCore.h"
#include "EditorCore/Localization.h"

namespace NN::Editor
{
	void PreferencesPanel::OnGUI()
	{
		if (m_IsOpen == false)
			return;

		ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);

		//ImGui::SetNextWindowClass(&EditorPreferences::GetImGuiWindowClass());
		if (ImGui::Begin(GetWindowFullName().c_str(), &m_IsOpen, ImGuiWindowFlags_NoCollapse))
		{
			// Left
			{
				ImGui::BeginChild("PreferencesPanelLeft", ImVec2(150, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
				auto& nameList = EditorCore::GetEditorPreferences().GetPreferencesNameList();

				for (int i = 0;i < nameList.size(); i++)
				{
					if (ImGui::Selectable(EditorText{ nameList[i]}.c_str(), m_SelectedPanelIndex == i))
					{
						m_SelectedPanelIndex = i;
					}
				}

				ImGui::EndChild();
			}

			ImGui::SameLine();

			// Right
			m_SelectedPanel = EditorCore::GetEditorPreferences().GetPreferencesByIndex(m_SelectedPanelIndex);
			{
				//ImGui::BeginGroup();
				ImGui::BeginChild("PreferencesPanelRight", ImVec2(0, -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us
				if (m_SelectedPanel)
				{
					m_SelectedPanel->OnGUI();
				}
				ImGui::EndChild();
				//ImGui::EndGroup();
			}


		}
		//EditorPreferences::GetInstance()->OnGUI();
		ImGui::End();

		// 保存偏好设置
		if (m_IsOpen == false)
		{
			EditorCore::GetEditorPreferences().Save(EditorCore::GetEditorPreferences());
		}
	}

	std::string PreferencesPanel::GetWindowFullName()
	{
		return EditorText{GetWindowName()}.GetText();
	}

	std::string PreferencesPanel::GetWindowName()
	{
		return "Editor Preferences";
	}

	void PreferencesPanel::OpenWindow(bool open)
	{
		m_IsOpen = open;
	}

	bool PreferencesPanel::IsWindowOpened()
	{
		return m_IsOpen;
	}

}
