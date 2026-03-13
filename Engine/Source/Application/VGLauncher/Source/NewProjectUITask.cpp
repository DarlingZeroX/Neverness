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

#include "NewProjectUITask.h"
#include "VGLauncherData.h"
#include "ProjectCreator.h"
#include <HCorePlatform/Include/NativeFileDialog/portable-file-dialogs.h>
#include <VGEditorFramework/Include/EditorCore/Localization.h>

namespace VisionGal::Editor
{
	// 构造函数，初始化项目名称和默认路径，并尝试恢复上次创建项目的目录
	NewProjectUITask::NewProjectUITask()
	{
		m_ProjectName = "New Project";
		m_ProjectLocation = "D:/";

		auto& data = VGLauncherData::GetLauncherData();

		// 尝试恢复上次创建项目的目录
		if (data.GetLastProjectCreateDirectory().empty() == false)
		{
			m_ProjectLocation = data.GetLastProjectCreateDirectory();
		}
	}

	void NewProjectUITask::TipText(const std::string& tip)
	{
		ImGui::TextColored(
			ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
			EditorText{ tip }.c_str()
		);
	}

	// 渲染新建项目的UI界面
	void NewProjectUITask::RenderUI(TaskContext& context)
	{
		// 如果任务被强制停止则直接返回
		if (context.ForceStop)
			return;

		auto& data = VGLauncherData::GetLauncherData();

		// 打开“新建项目”弹窗
		ImGui::OpenPopup(EditorText{ "New Project" }.c_str());

		// 弹窗居中显示
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		bool hasChoice = false;

		// 渲染弹窗内容
		if (ImGui::BeginPopupModal(EditorText{ "New Project" }.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			bool isValidProject = true;

			// 项目名称输入框
			ImGui::Text(EditorText{ "Project name" }.c_str());
			ImGuiEx::InputText("##Project name", m_ProjectName);

			// 检查项目名称是否为空
			if (m_ProjectName.empty())
			{
				TipText("Project name cannot be empty!");
				isValidProject = false;
			}

			// 项目路径输入框
			ImGui::Text(EditorText{ "Project location" }.c_str());
			ImGuiEx::InputText("##Project location", m_ProjectLocation);

			// 选择项目路径按钮
			ImGui::SameLine();
			if (ImGui::Button(ICON_FA_FOLDER "##OpenProjectLocation"))
			{
				OpenProjectLocationDialog();
			}

			// 检查项目路径是否合法
			if (m_ProjectLocation.empty() || !std::filesystem::exists(m_ProjectLocation))
			{
				TipText("Invalid project location!");
				isValidProject = false;
			}

			// 检查同名项目是否已存在
			std::filesystem::path path = m_ProjectLocation;
			if (std::filesystem::exists(path / m_ProjectName))
			{
				TipText("The project already exists!");
				isValidProject = false;
			}

			// 创建项目按钮
			if (ImGui::Button(EditorText{ "Create project" }.c_str(), ImVec2(120, 0)))
			{
				if (isValidProject)
				{
					CreateProject();
					hasChoice = true;
					data.SetLastProjectCreateDirectory(m_ProjectLocation);
					ImGui::CloseCurrentPopup();
				}
			}
			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			// 取消按钮
			if (ImGui::Button(EditorText{ "Cancel" }.c_str(), ImVec2(120, 0)))
			{
				hasChoice = true;
				data.SetLastProjectCreateDirectory( m_ProjectLocation );
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		// 用户已做出选择，标记任务完成
		if (hasChoice)
			context.IsFinished = true;
	}

	// 打开文件夹选择对话框，选择项目存储路径
	bool NewProjectUITask::OpenProjectLocationDialog()
	{
		auto& data = VGLauncherData::GetLauncherData();

		auto root = data.GetLastProjectCreateDirectory();
		auto selection = pfd::select_folder(EditorText{ "Open Project" }.c_str(), root).result();

		if (selection.empty())
			return false;

		m_ProjectLocation = selection;
		return true;
	}

	// 创建项目并弹出通知
	void NewProjectUITask::CreateProject()
	{
		auto& data = VGLauncherData::GetLauncherData();

		ProjectCreateConfig config;
		config.ProjectName = m_ProjectName;
		config.ProjectPath = m_ProjectLocation;

		// 尝试创建项目并根据结果弹出通知
		if (ProjectCreator::CreateProject(config))
		{
			data.SetLastProjectCreateDirectory(config.ProjectPath);
			ImGuiEx::PushNotification({ ImGuiExToastType::Info, EditorText{ "Create project successfully" }.c_str() });
		}
		else
		{
			ImGuiEx::PushNotification({ ImGuiExToastType::Error, EditorText{ "Create project failed" }.c_str() });
		}
	}

}
