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
#include "VGPackageTool.h"
#include <VGImgui/IncludeImGui.h>
#include <VGEditorCore/Include/EditorCore/Localization.h>

namespace VisionGal::Editor
{
	VGPackageToolMainWindow::VGPackageToolMainWindow()
	{
	}

	void VGPackageToolMainWindow::OnGUI()
	{
		// 设置窗口样式
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar
			| ImGuiWindowFlags_NoDecoration
			| ImGuiWindowFlags_NoMove
			| ImGuiWindowFlags_NoSavedSettings
			| ImGuiWindowFlags_NoBringToFrontOnFocus;

		// 设置窗口位置和大小
		bool use_work_area = true;
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(use_work_area ? viewport->WorkPos : viewport->Pos);
		ImGui::SetNextWindowSize(use_work_area ? viewport->WorkSize : viewport->Size);

		// 开始绘制窗口
		if (ImGui::Begin("VGPackageTool Main Editor", nullptr, flags))
		{
			OnGUIInternal();
		}
		ImGui::End();
	}

	void VGPackageToolMainWindow::OnUpdate(float delta)
	{
	}

	void VGPackageToolMainWindow::OnFixedUpdate()
	{
	}

	std::string VGPackageToolMainWindow::GetWindowFullName()
	{
		return {};
	}

	std::string VGPackageToolMainWindow::GetWindowName()
	{
		return {};
	}

	void VGPackageToolMainWindow::OpenWindow(bool open)
	{
	}

	bool VGPackageToolMainWindow::IsWindowOpened()
	{
		return false;
	}

	void VGPackageToolMainWindow::OnGUIInternal()
	{
		// 显示启动器左边的按钮UI
		{
			ImGui::BeginChild("VGPackageTool left pane", ImVec2(150, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);

			RenderLeftUI();

			ImGui::EndChild();
		}

		ImGui::SameLine();

		// 显示启动器右边的项目列表UI
		{
			ImGui::BeginGroup();
			ImGui::BeginChild("VGPackageTool item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us

			RenderRightUI();

			ImGui::EndChild();
			ImGui::EndGroup();
		}
	}

	void VGPackageToolMainWindow::RenderLeftUI()
	{
		// 设置左侧按钮的样式
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.33f, 0.33f, 0.33f, 1.00f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.44f, 0.44f, 0.44f, 1.00f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.24f, 0.24f, 0.24f, 0.4f });
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4{ 0.0f, 0.0f, 0.0f, 0.01f });

		// 添加一个空白的占位符，确保按钮在左侧对齐
		ImGui::Dummy(ImVec2(0, 25));

		ImVec2 buttonSize = ImVec2(ImGui::GetContentRegionAvail().x, 40.f);

		if (ImGui::Button(EditorText{ "Packaging Engine Editor Resources" }.c_str(), buttonSize))
		{
			VGPackageTool::PakEngineEditorResources();
		}


		ImGui::PopStyleColor(4);
	}

	void VGPackageToolMainWindow::RenderRightUI()
	{
	}
}
