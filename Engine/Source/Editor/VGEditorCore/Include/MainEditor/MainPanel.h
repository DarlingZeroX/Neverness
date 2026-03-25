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
#include <functional>
#include "../Config.h"
#include <vector>
#include <unordered_map>
#include <HCore/Interface/HConfig.h>
#include <VGImgui/IncludeImGui.h>

namespace VisionGal::Editor
{
	class VG_EDITOR_FRAMEWORK_API EditorMainWindow : public IEditorPanel
	{
	public:
		EditorMainWindow();
		EditorMainWindow(const EditorMainWindow&) = default;
		EditorMainWindow& operator=(const EditorMainWindow&) = default;
		EditorMainWindow(EditorMainWindow&&) noexcept = default;
		EditorMainWindow& operator=(EditorMainWindow&&) noexcept = default;
		~EditorMainWindow() override = default;
		 
		// 通过 IPanel 继承
		void OnGUI() override;
		void OnUpdate(float delta) override;
		void OnFixedUpdate() override;
		std::string GetWindowFullName() override;
		std::string GetWindowName() override;
		void OpenWindow(bool open) override;
		bool IsWindowOpened() override;

		void AddPanel(Ref<IEditorPanel> panel);
		void AddPanelWithID(const std::string& id,Ref<IEditorPanel> panel);

		void RequestRearrangeLayout();
		void LoadDefaultLayout();

		void TraversePanels(std::function<void(IEditorPanel* panel)> callback);

		IEditorPanel* GetPanelWithID(const std::string& id);
	private:
		bool m_bItemDragging;
		ImGuiWindowClass m_WindowClass;

		std::vector<Ref<IEditorPanel>> m_Panels;
		std::unordered_map<std::string, Ref<IEditorPanel>> m_IDPanels;

		bool m_NeedInitializeWindowSize = false;
		ImVec2 m_InitializeWindowSize;

		ImGuiWindowClass m_DockSpaceWindowClass;
		bool m_DockspaceLayoutInitialized = false;
		bool m_DockspaceLayoutRequestRearrange = false;
	};


}
