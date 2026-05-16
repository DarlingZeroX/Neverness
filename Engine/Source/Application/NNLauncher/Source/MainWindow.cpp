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

#include "MainWindow.h"
#include "VGLauncherData.h"
#include "NewProjectUITask.h"
#include <filesystem>
#include <NNPlatformCore/Include/NativeFileDialog/portable-file-dialogs.h>
#include <NNRuntimeImGui/Include/ImGuiEx/IconFont/IconsFontAwesome5Pro.h>
#include <NNEngineLegacy/Include/Engine/VGEngine.h>
#include <NNEditorFramework/Include/EditorCore/Localization.h>

namespace VisionGal::Editor
{	using namespace NN::Editor;
using namespace NN::Runtime;
	VGLauncherMainWindow::VGLauncherMainWindow()
	{
		m_ProjectItemSize.y = 63.0f;;
		m_SelectProject = -1;
	}

	void VGLauncherMainWindow::OnGUI()
	{
		// 设置窗口样式
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar
			| ImGuiWindowFlags_NoDecoration
			| ImGuiWindowFlags_NoMove
			| ImGuiWindowFlags_NoSavedSettings
			| ImGuiWindowFlags_NoBringToFrontOnFocus;

		// 设置窗口位置和大小，使其覆盖整个视口
		bool use_work_area = true;
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(use_work_area ? viewport->WorkPos : viewport->Pos);
		ImGui::SetNextWindowSize(use_work_area ? viewport->WorkSize : viewport->Size);

		// 开始绘制窗口
		if (ImGui::Begin(GetWindowFullName().c_str(), nullptr, flags))
		{
			OnGUIInternal();
		}
		ImGui::End();
	}

	void VGLauncherMainWindow::OnUpdate(float delta)
	{
	}

	void VGLauncherMainWindow::OnFixedUpdate()
	{
	}

	std::string VGLauncherMainWindow::GetWindowFullName()
	{
		return GetWindowName();
	}

	std::string VGLauncherMainWindow::GetWindowName()
	{
		return "VGLauncher";
	}

	void VGLauncherMainWindow::OpenWindow(bool open)
	{
	}
	bool VGLauncherMainWindow::IsWindowOpened()
	{
		return true;
	}

	void VGLauncherMainWindow::OnGUIInternal()
	{
		// 显示启动器左边的按钮UI
		{
			ImGui::BeginChild("left pane", ImVec2(150, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
			RenderLeftUI();
			ImGui::EndChild();
		}

		ImGui::SameLine();

		// 显示启动器右边的项目列表UI
		{
			ImGui::BeginGroup();
			ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us
			RenderRightUI();
			ImGui::EndChild();
			ImGui::EndGroup();
		}
	}

	bool VGLauncherMainWindow::LeftButtonUI(const std::string& title, const std::string& icon)
	{
		return ImGui::Button(
			EditorText{ title , icon }.c_str(),
			ImVec2(ImGui::GetContentRegionAvail().x, 40.f)
		);
	}

	void VGLauncherMainWindow::RenderLeftUI()
	{
		// 设置左侧按钮的样式
		//ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 8));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.33f, 0.33f, 0.33f, 1.00f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.44f, 0.44f, 0.44f, 1.00f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.24f, 0.24f, 0.24f, 0.4f });
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4{ 0.0f, 0.0f, 0.0f, 0.01f });

		// 添加一个空白的占位符，确保按钮在左侧对齐
		ImGui::Dummy(ImVec2(0, 25));

		// 打开项目按钮
		if (LeftButtonUI("Open project", ICON_FA_FOLDER))	{
			OpenProjectDialog();
		}

		// 新建项目按钮
		if (LeftButtonUI("New project" ,ICON_FA_PLUS))	{
			auto* uiTask = new NewProjectUITask();
			auto task = ImGuiEx::NewUITask(uiTask, "New Directory");
		}

		// 移除项目按钮
		if (LeftButtonUI("Remove project" ,ICON_FA_TIMES))	{
			RemoveProject(m_SelectProject);
		}

		// 编辑项目按钮
		if (LeftButtonUI("Edit project", ICON_FA_EDIT))	{
			EditProject(m_SelectProject);
		}

		ImGui::PopStyleColor(4);
		//ImGui::PopStyleVar();
	}

	void VGLauncherMainWindow::RenderRightUI()
	{
		if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
		{
			if (ImGui::BeginTabItem(EditorText{ "Projects" }.c_str()))
			{
				// 获取项目列表
				auto& projects = VGLauncherData::GetLauncherData().GetProjects();

				for (int i = 0;i < projects.size(); i++)
				{
					ImGui::PushID(i);
					auto& project = projects[i];

					// 设置项目项UI的大小
					ImVec2 itemSize;
					itemSize.x = ImGui::GetContentRegionAvail().x;
					itemSize.y = m_ProjectItemSize.y;

					// 创建一个不可见按钮用于检测点击和悬停
					ImGui::InvisibleButton(project.Path.c_str(), itemSize);

					if (ImGui::IsItemHovered())
					{
						// 鼠标左键点击时选择项目
						if (ImGui::IsItemClicked())
						{
							m_SelectProject = i;
						}

						// 鼠标双击时启动项目
						if (ImGui::IsMouseDoubleClicked(0))
						{
							EditProject(i);
						}
					}

					ImGui::PopID();

					// 渲染项目项
					RenderProjectItem(i, project.Name, project.Path);
				}

				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}

	void VGLauncherMainWindow::RenderProjectItem(int index, const std::string& name, const std::string& path)
	{
		ImVec2 itemSize;
		itemSize.x = ImGui::GetContentRegionAvail().x;
		itemSize.y = m_ProjectItemSize.y;

		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		const ImVec2 p0 = ImGui::GetItemRectMin();
		const ImVec2 p1 = ImVec2(p0.x + itemSize.x, p0.y + itemSize.y);

		// 计算项目名称和路径文本区域的位置
		const ImVec2 namePosStart = ImVec2(p0.x + 20, p0.y + 5);
		const ImVec2 pathPosStart = ImVec2(p0.x + 20, namePosStart.y + 30);

		// 项目被选中添加边框和背景颜色
		if (m_SelectProject == index)
		{
			draw_list->AddRectFilled(p0, p1, IM_COL32(70, 70, 70, 255), 10.0f);
			draw_list->AddRect(p0, p1, IM_COL32(173, 216, 230, 200), 10, 0, 3);
		}
		else
		{
			draw_list->AddRectFilled(p0, p1, IM_COL32(50, 50, 50, 255), 10.0f);
		}

		// 添加剪裁矩形以限制绘制区域
		ImGui::PushClipRect(p0, p1, true);
		auto font = ImGui::GetFont();

		// 项目名称
		draw_list->AddText(font, 30, namePosStart, IM_COL32_WHITE, name.c_str());
		// 项目路径
		draw_list->AddText(font, 20, pathPosStart, IM_COL32(165, 165, 165, 255), path.c_str());

		// 鼠标悬停时添加边框
		if (ImGui::IsItemHovered())
			draw_list->AddRect(p0, p1, IM_COL32(100, 100, 100, 200), 10, 0, 3);

		ImGui::PopClipRect();
	}

	void VGLauncherMainWindow::OpenProjectDialog()
	{
		auto root = "";

		// 打开文件夹选择对话框
		auto selection = pfd::select_folder(EditorText{ "Open Project" }.c_str(), root).result();
		if (selection.empty())
			return;

		auto& data = VGLauncherData::GetLauncherData();

		// 添加项目到项目列表
		VGProjectItem projectItem;
		projectItem.Name = selection.substr(selection.find_last_of("/\\") + 1);
		projectItem.Path = selection;
		data.AddProject(projectItem);
	}

	bool VGLauncherMainWindow::RemoveProject(int index)
	{
		auto& projectsData = VGLauncherData::GetLauncherData();

		if (index < 0)
			return false;

		if (index >= projectsData.GetProjects().size())
			return false;

		// 删除项目
		projectsData.RemoveProject(index);

		m_SelectProject = 0;

		return true;
	}

	bool VGLauncherMainWindow::EditProject(int index)
	{
		namespace fs = std::filesystem;
		auto& projects = VGLauncherData::GetLauncherData().GetProjects();

		if (index < 0)
			return false;

		if (index >= projects.size())
			return false;

		auto& project = projects[index];

		// 创建一个线程来启动编辑器
		std::thread thread([project]()
			{
				fs::path editorPath = fs::current_path() / "VGEditor";
#ifdef _WIN32
				editorPath += ".exe";
#endif

				// 写入编辑器启动的项目路径到 VGLauncher.txt
				NN::Core::HFileSystem::CreateDirectoryWhenNoExist("Data");
				NN::Core::HFileSystem::WriteTextToFile("Data/EditorStartupData.txt", project.Path);

				// 使用 std::system 启动编辑器
				std::string command = "\"" + editorPath.string();
				std::system(command.c_str());  // 简单，但不能获得进程句柄
			});

		// 分离线程以便它可以在后台运行
		thread.detach();

		// 等待一段时间以确保线程启动编辑器
		std::this_thread::sleep_for(std::chrono::duration<double>(0.5));

		// 退出启动器程序
		VGEngine::Get()->RequestExit();

		return true;
	}
}
