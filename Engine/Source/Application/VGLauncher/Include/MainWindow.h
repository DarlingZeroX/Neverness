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

#pragma once
#include <VGImgui/IncludeImGui.h>
#include <VGEditorCore/Interface/UIInterface.h>

namespace VisionGal::Editor
{
	/// @brief VGLauncher 主窗口类，用于显示和管理项目列表
	class VGLauncherMainWindow : public IEditorPanel
	{
	public:
		VGLauncherMainWindow();
		VGLauncherMainWindow(const VGLauncherMainWindow&) = default;
		VGLauncherMainWindow& operator=(const VGLauncherMainWindow&) = default;
		VGLauncherMainWindow(VGLauncherMainWindow&&) noexcept = default;
		VGLauncherMainWindow& operator=(VGLauncherMainWindow&&) noexcept = default;
		~VGLauncherMainWindow() override = default;

		// 通过 IEditorPanel 继承
		void OnGUI() override;
		void OnUpdate(float delta) override;
		void OnFixedUpdate() override;
		std::string GetWindowFullName() override;
		std::string GetWindowName() override;
		void OpenWindow(bool open) override;
		bool IsWindowOpened() override;

		/// @brief 移除一个项目
		/// @param index 索引
		bool RemoveProject(int index);
		/// @brief 编辑一个项目
		/// @param index 索引
		bool EditProject(int index);
		/// @brief 打开项目对话框
		void OpenProjectDialog();
	private:
		static bool LeftButtonUI(const std::string& title, const std::string& icon);
		void OnGUIInternal();
		void RenderLeftUI();
		void RenderRightUI();
		void RenderProjectItem(int index, const std::string& name, const std::string& path);

		ImVec2 m_ProjectItemSize;
		int m_SelectProject;
	};


}
