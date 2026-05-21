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
#include <NNEditorFrameworkLegacy/Interface/UIInterface.h>
#include <HNGEditorCore/Interface/NodeGraphEditor.h>

#include "VGEditorGalgame/Module.h"
#include "VGEditorGalgame/Interface/VisualGalgame.h"

namespace VisionGal::Editor
{
	/// @brief VGLauncher 主窗口类，用于显示和管理项目列表
	class VGNodeGraphMainWindow : public IEditorPanel
	{
	public:
		VGNodeGraphMainWindow();
		VGNodeGraphMainWindow(const VGNodeGraphMainWindow&) = default;
		VGNodeGraphMainWindow& operator=(const VGNodeGraphMainWindow&) = default;
		VGNodeGraphMainWindow(VGNodeGraphMainWindow&&) noexcept = default;
		VGNodeGraphMainWindow& operator=(VGNodeGraphMainWindow&&) noexcept = default;
		~VGNodeGraphMainWindow() override = default;

		// 通过 IEditorPanel 继承
		void OnGUI() override;
		void OnUpdate(float delta) override;
		void OnFixedUpdate() override;
		std::string GetWindowFullName() override;
		std::string GetWindowName() override;
		void OpenWindow(bool open) override;
		bool IsWindowOpened() override;

	private:
		void OnGUIInternal();
		void RenderLeftUI();
		void RenderRightUI();
	private:
		//Horizon::NodeGraph::HNodeGraphEditor m_NodeGraphEditor;
		VisualGalgame m_NodeGraphEditor;

		VisualGalEditor m_VisualGalEditor;

		//Horizon::NodeGraph::NodeGraphWindow m_NodeGraphWindow{ "VGPackageTool Node Graph" };
	};
}
