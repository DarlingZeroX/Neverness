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
#include <VGImgui/IncludeImGui.h>
#include <VGEditorCore/Include/EditorCore/Localization.h>

namespace VisionGal::Editor
{
	VGNodeGraphMainWindow::VGNodeGraphMainWindow()
	{
	}

	void VGNodeGraphMainWindow::OnGUI()
	{
		// 设置窗口样式
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar
			| ImGuiWindowFlags_NoDecoration
			| ImGuiWindowFlags_NoMove
			| ImGuiWindowFlags_NoSavedSettings
			| ImGuiWindowFlags_NoBringToFrontOnFocus;

		// 设置窗口位置和大小
		//bool use_work_area = true;
		//const ImGuiViewport* viewport = ImGui::GetMainViewport();
		//ImGui::SetNextWindowPos(use_work_area ? viewport->WorkPos : viewport->Pos);
		//ImGui::SetNextWindowSize(use_work_area ? viewport->WorkSize : viewport->Size);

		// 开始绘制窗口
		if (ImGui::Begin("VGNodeGraphApp Main Editor", nullptr))
		{
			OnGUIInternal();
		}
		ImGui::End();
	}

	void VGNodeGraphMainWindow::OnUpdate(float delta)
	{
	}

	void VGNodeGraphMainWindow::OnFixedUpdate()
	{
	}

	std::string VGNodeGraphMainWindow::GetWindowFullName()
	{
		return {};
	}

	std::string VGNodeGraphMainWindow::GetWindowName()
	{
		return {};
	}

	void VGNodeGraphMainWindow::OpenWindow(bool open)
	{
	}

	bool VGNodeGraphMainWindow::IsWindowOpened()
	{
		return false;
	}

	void VGNodeGraphMainWindow::OnGUIInternal()
	{
		m_NodeGraphEditor.DrawNodeGraphWindow();
	}
}
