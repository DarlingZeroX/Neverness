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

#include "Setting/ProjectSettingsPanel.h"
#include "Setting/ProjectSettingsWrapper.h"
#include "EditorCore/Localization.h"
#include <NNRuntimeImGui/IncludeImGuiEx.h>

#include "NNEngineLegacy/Include/Project/ProjectSettings.h"

namespace NN::Editor
{
	void ProjectSettingPanel::OnGUI()
	{
		if (m_IsOpen == false)
			return;

		ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);

		//ImGui::SetNextWindowClass(&EditorPreferences::GetImGuiWindowClass());
		if (ImGui::Begin(GetWindowFullName().c_str(), &m_IsOpen, ImGuiWindowFlags_NoCollapse))
		{
			// Left
			{
				ImGui::BeginChild("ProjectSettingPanelLeft", ImVec2(150, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
				auto& nameList = ProjectSettingsWrapper::GetProjectSettingsNameList();

				for (int i = 0; i < nameList.size(); i++)
				{
					if (ImGui::Selectable(EditorText{ nameList[i] }.c_str(), m_SelectedPanelIndex == i))
					{
						m_SelectedPanelIndex = i;
					}
				}

				ImGui::EndChild();
			}

			ImGui::SameLine();

			// Right
			m_SelectedPanel = ProjectSettingsWrapper::GetProjectSettingsByIndex(m_SelectedPanelIndex);
			{
				//ImGui::BeginGroup();
				ImGui::BeginChild("ProjectSettingPanelRight", ImVec2(0, -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us
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

		// 保存项目设置
		if (m_IsOpen == false)
		{
			Runtime::ProjectSettings::SaveProjectSettings();
			//EditorCore::GetEditorPreferences().Save(EditorCore::GetEditorPreferences());
		}
	}

	std::string ProjectSettingPanel::GetWindowFullName()
	{
		return EditorText{ GetWindowName() }.GetText();
	}

	std::string ProjectSettingPanel::GetWindowName()
	{
		return "Project Setting";
	}

	void ProjectSettingPanel::OpenWindow(bool open)
	{
		m_IsOpen = open;
	}

	bool ProjectSettingPanel::IsWindowOpened()
	{
		return m_IsOpen;
	}

}
