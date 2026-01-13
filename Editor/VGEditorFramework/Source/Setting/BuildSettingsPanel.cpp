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

#include "Setting/BuildSettingsPanel.h"
#include "EditorCore/Localization.h"
#include <VGImgui/IncludeImGuiEx.h>
#include "HCore/Include/Platform/NativeFileDialog/portable-file-dialogs.h"
#include "HCore/Include/System/HFileSystem.h"
#include "VGEngine/Include/Asset/Package.h"
#include "VGEngine/Include/Project/ProjectSettings.h"
#include "VGEngine/Include/Project/ProjectBuilder.h"

namespace VisionGal::Editor
{
	BuildSettingsPanel::BuildSettingsPanel()
	{
		m_TargetPlatforms = {
			"Windows",
			//"Linux",
			//"MacOS",
			//"Android",
			//"iOS"
		};
	}

	void BuildSettingsPanel::OnGUI()
	{
		if (m_IsOpen == false)
			return;

		ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);

		//ImGui::SetNextWindowClass(&EditorPreferences::GetImGuiWindowClass());
		if (ImGui::Begin(GetWindowFullName().c_str(), &m_IsOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize))
		{
			OnGUIInternal();
			RenderBuildProcess();
		}
		ImGui::End();

		// 保存项目设置
		if (m_IsOpen == false)
		{
			ProjectSettings::SaveProjectSettings();
		}
	}

	std::string BuildSettingsPanel::GetWindowFullName()
	{
		return EditorText{ GetWindowName() }.GetText();
	}

	std::string BuildSettingsPanel::GetWindowName()
	{
		return "Build Setting";
	}

	void BuildSettingsPanel::OpenWindow(bool open)
	{
		m_IsOpen = open;

		if (m_IsOpen)
		{
			// 设置项目主场景
			auto& projectSettings = ProjectSettings::GetProjectSettings();
			if (projectSettings.Application.RunningMainScene.empty())
			{
				projectSettings.Application.RunningMainScene = projectSettings.Editor.MainScene;
			}

			m_ApplicationMainScene = ProjectSettings::GetProjectSettings().Application.RunningMainScene;
		}
	}

	bool BuildSettingsPanel::IsWindowOpened()
	{
		return m_IsOpen;
	}


	bool BuildSettingsPanel::ApplicationMainSceneUI()
	{
		ImVec4 invalidTextColor = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);

		// 项目主场景输入框
		ImGui::Text(EditorText{ "Application main scene" }.c_str());
		//ImGui::SameLine();
		ImGuiEx::InputText("##Application main scene", m_ApplicationMainScene);

		if (ImGui::BeginDragDropTarget())
		{
			if (const auto* payload = ImGui::AcceptDragDropPayload("PLACE_CONTENT_BROWSER_ITEM"))
			{
				std::string data = static_cast<char*>(payload->Data);
				m_ApplicationMainScene = data;
				//ImGuiEx::PushNotification({ ImGuiExToastType::Info, "Drop Video" });
			}
			ImGui::EndDragDropTarget();
		}

		// 检查项目名称是否为空
		if (m_ApplicationMainScene.empty())
		{
			ImGui::TextColored(invalidTextColor, EditorText{ "Application main scene is invalid!" }.c_str());
			return false;
		}

		VGAssetMetaData data;
		if (VGPackage::GetMeatData(m_ApplicationMainScene, data))
		{
			if (data.AssetType != "Scene")
			{
				ImGui::TextColored(invalidTextColor, EditorText{ "Application main scene is invalid!" }.c_str());
				return false;
			}

			return true;
		}

		ImGui::TextColored(invalidTextColor, EditorText{ "Application main scene is invalid!" }.c_str());
		return false;
	}

	bool BuildSettingsPanel::BuildOutputDirectoryUI()
	{
		ImVec4 invalidTextColor = ImVec4(1.0f, 0.1f, 0.1f, 1.0f);

		// 生成目录
		ImGui::Text(EditorText{ "Build output directory" }.c_str());
		//ImGui::SameLine();
		ImGuiEx::InputText("##Build output directory", m_ProjectBuildOutputDirectory);

		// 选择项目路径按钮
		ImGui::SameLine();
		if (ImGui::Button(ICON_FA_FOLDER "##OpenBuildOutputDirectory"))
		{
			OpenBuildOutputDirectoryDialog();
		}

		// 检查生成目录是否为空
		if (m_ProjectBuildOutputDirectory.empty())
		{
			ImGui::TextColored(invalidTextColor, EditorText{ "Build output directory cannot be empty!" }.c_str());
			return false;
		}

		// 生成目录是否有效
		if (Horizon::HFileSystem::ExistsDirectory(m_ProjectBuildOutputDirectory) == false)
		{
			ImGui::TextColored(invalidTextColor, EditorText{ "Build output directory is invalid!" }.c_str());
			return false;
		}

		// 生成目录必须为空目录
		if (Horizon::HFileSystem::DirectoryEmpty(m_ProjectBuildOutputDirectory) == false)
		{
			ImGui::TextColored(invalidTextColor, EditorText{ "Build output directory must be empty directory!" }.c_str());
			return false;
		}

		return true;
	}

	void BuildSettingsPanel::OnGUIInternal()
	{
		// 项目主场景
		bool isValidBuildSetting = ApplicationMainSceneUI();
		if (isValidBuildSetting)
		{
			// 输出目录
			isValidBuildSetting = BuildOutputDirectoryUI();

			// 构建平台
			ImGui::Text(EditorText{ "Target Platform" }.c_str());
			if (ImGui::BeginCombo("##Target Platform", "Windows", ImGuiComboFlags_PopupAlignLeft))
			{
				for (int i = 0; i < m_TargetPlatforms.size(); i++)
				{
					if (ImGui::Selectable(m_TargetPlatforms[i].c_str()))
					{
					}
				}
				ImGui::EndCombo();
			}
		}

		// 构建按钮
		if (ImGui::Button(EditorText{ "Build" }.c_str(), ImVec2(120, 0)))
		{
			if (isValidBuildSetting)
			{
				auto& projectSettings = ProjectSettings::GetProjectSettings();
				projectSettings.Application.RunningMainScene = m_ApplicationMainScene;

				m_TargetPlatform = "Windows";
				m_BuildContext.OutputDirectory = m_ProjectBuildOutputDirectory;
				m_BuildContext.TargetPlatform = m_TargetPlatform; // 这里可以根据选择的目标平台进行设置
				//ProjectBuilder::BuildProject(m_ProjectBuildOutputDirectory, "Windows");
				std::thread thread([this]()
					{
						ProjectBuilder::BuildProject(m_BuildContext);
					});
				thread.detach();

				ImGui::OpenPopup("Build Process");
			}
		}
		ImGui::SameLine();
		// 取消按钮
		if (ImGui::Button(EditorText{ "Cancel" }.c_str(), ImVec2(120, 0)))
		{
			m_IsOpen = false;
		}

	}

	void BuildSettingsPanel::RenderBuildProcess()
	{
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		if (ImGui::BeginPopupModal("Build Process", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar))
		{
			ImGui::Text(EditorText{ "Building project..." }.c_str());
			ImGui::Separator();
			// 显示构建进度

			std::string processText = std::to_string(static_cast<int>(m_BuildContext.process * 100)) + "%";
			ImGui::ProgressBar(m_BuildContext.process, ImVec2(0.0f, 0.0f), processText.c_str());

			if (m_BuildContext.process == 1.0)
			{
				ImGui::Text(EditorText{ "Build successfully!" }.c_str());
			}

			if (m_BuildContext.isError == true)
			{
				ImGui::Text(EditorText{ "Build failed!" }.c_str());
			}

			if (ImGui::Button(EditorText{ "Close" }.c_str()))
			{
				ImGui::CloseCurrentPopup();
				m_IsOpen = false;
			}

			ImGui::EndPopup();
		}
	}


	void BuildSettingsPanel::OpenBuildOutputDirectoryDialog()
	{
		auto selection = pfd::select_folder(EditorText{ "Open Project" }.c_str()).result();

		if (selection.empty())
			return;

		m_ProjectBuildOutputDirectory = selection;
	}

}
