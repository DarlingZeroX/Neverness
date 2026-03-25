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

#include "MainEditor/MainPanel.h"
#include "PanelManager.h"
#include "EditorCore/Localization.h"
#include <VGImgui/IncludeImGuiEx.h>

#include "EditorCore/EdtiorScene.h"
#include "VGEngine/Include/Engine/Manager.h"
#include "VGEngine/Include/Engine/Manager/SceneManager.h"


namespace VisionGal::Editor
{
	EditorMainWindow::EditorMainWindow()
	{
		m_DockSpaceWindowClass.ClassId = ImHashStr("Scene Editor Window");
	}

	void EditorMainWindow::OnGUI()
	{
		auto* editor = PanelManager::GetInstance();

		// Submit top-level documents
		ImGui::SetNextWindowClass(&editor->GetImGuiWindowClass());

		bool use_work_area = true;
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar
		| ImGuiWindowFlags_NoDecoration
		| ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoSavedSettings
		| ImGuiWindowFlags_NoBringToFrontOnFocus;

		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(use_work_area ? viewport->WorkPos : viewport->Pos);
		ImGui::SetNextWindowSize(use_work_area ? viewport->WorkSize : viewport->Size);

		bool open;
		if (open = ImGui::Begin("Main Editor", nullptr, flags))
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 8));
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.1f, 0.1f, 0.1f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 1.0f, 1.0f, 1.0f, 0.2f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 1.0f, 1.0f, 1.0f, 0.4f });
			ImGui::PushStyleColor(ImGuiCol_Border, ImVec4{ 0.0f, 0.0f, 0.0f, 0.01f });

			if (ImGui::Button(EditorText{ "Save Scene", ICON_FA_SAVE }.c_str()))
			{
				EditorScene::SaveCurrentScene();
			}

			ImGui::SameLine();

			/*
			// Using the generic BeginCombo() API, you have full control over how to display the combo contents.
			// (your selection data could be an index, a pointer to the object, an id for the object, a flag intrusively
			// stored in the object itself, etc.)
			const char* items[] = {
				"Select",
				"Landscape",
				"Foliage",
				//	"Mesh Paint",
				//	"Modeling",
				//	"Fracture",
				//	"Brush Editing",
				//	"Animation"
			};
			static int selectWindowCurrentIndex = 2; // Here we store our selection data as an index.
			const char* combo_preview_value = items[selectWindowCurrentIndex];  // Pass in the preview value visible before opening the combo (it could be anything)

			ImGui::SetNextItemWidth(180);

			if (ImGui::BeginCombo("##combo 1", combo_preview_value, ImGuiComboFlags_PopupAlignLeft))
			{
				for (int n = 0; n < IM_ARRAYSIZE(items); n++)
				{
					const bool is_selected = (selectWindowCurrentIndex == n);
					if (ImGui::Selectable(items[n], is_selected))
						selectWindowCurrentIndex = n;

					// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			ImGui::SameLine();
			*/

			// 让按钮居中
			{
				float windowWidth = ImGui::GetWindowSize().x;
				float x = (windowWidth) * 0.5f;
				ImGui::SetCursorPosX(x);
			}

			// 播放/停止场景按钮
			if (GetSceneManager()->IsPlayMode() == false)
			{
				if (ImGui::Button(ICON_FA_PLAY "##ScenePlay"))
				{
					GetSceneManager()->EnterPlayMode();
				}
			} 
			else
			{
				if (ImGui::Button(ICON_FA_STOP "##SceneStop"))
				{
					GetSceneManager()->ExitPlayMode(); 
				}
			}

			ImGui::PopStyleColor(4);
			ImGui::PopStyleVar();

			ImGui::DockSpace(m_DockSpaceWindowClass.ClassId, ImVec2(0, 0), 0, &m_DockSpaceWindowClass);

			LoadDefaultLayout();

			for (auto& panel : m_Panels)
			{
				// Submit tools
				ImGui::SetNextWindowClass(&m_DockSpaceWindowClass);
				panel->OnGUI();
			}

			for (auto& pair : m_IDPanels)
			{
				auto& panel = pair.second;
				// Submit tools
				ImGui::SetNextWindowClass(&m_DockSpaceWindowClass);
				panel->OnGUI();
			}

			//	if (selectWindowCurrentIndex > 0)
			//		m_SelectPanels[selectWindowCurrentIndex - 1]->OnGUI();
		}

		ImGui::End();
	}

	void EditorMainWindow::OnUpdate(float delta)
	{
		for (auto& panel: m_Panels)
		{
			panel->OnUpdate(delta);
		}

		for (auto& [name, panel] : m_IDPanels)
		{
			panel->OnUpdate(delta);
		}
	}

	void EditorMainWindow::OnFixedUpdate()
	{

	}

	std::string EditorMainWindow::GetWindowFullName()
	{
		return "Main Editor Window";
	}

	std::string EditorMainWindow::GetWindowName()
	{
		return "Main Editor Window";
	}

	void EditorMainWindow::OpenWindow(bool open)
	{
		
	}

	bool EditorMainWindow::IsWindowOpened()
	{
		return true; // Always opened
	}

	void EditorMainWindow::AddPanel(Ref<IEditorPanel> panel)
	{
		m_Panels.push_back(panel);
	}

	void EditorMainWindow::AddPanelWithID(const std::string& id, Ref<IEditorPanel> panel)
	{
		m_IDPanels[id] = panel;
	}

	void EditorMainWindow::RequestRearrangeLayout()
	{
		m_DockspaceLayoutRequestRearrange = true;
	}

	void EditorMainWindow::LoadDefaultLayout()
	{
		// 检查是否需要重新布局
		if (m_DockspaceLayoutRequestRearrange == false)
		{
			// 如果不需要重新布局，则检查是否已经初始化过布局
			if (m_DockspaceLayoutInitialized)
			{
				return;
			}

			// 检查是否存在 imgui.ini 文件
			if (std::filesystem::exists("imgui.ini")) {
				m_DockspaceLayoutInitialized = true;
				return;
			}
		}

		ImGuiID dockSpaceID = m_DockSpaceWindowClass.ClassId;
		if (ImGui::DockBuilderGetNode(dockSpaceID))
		{
			// 清除现有的 dock
			ImGui::DockBuilderRemoveNode(dockSpaceID);
			ImGui::DockBuilderAddNode(dockSpaceID, ImGuiDockNodeFlags_None);
			//ImGui::DockBuilderSetNodeSize(dockSpaceID, viewport->Size);

			// 创建 dock 节点
			ImGuiID dock_main_id = dockSpaceID;
			ImGuiID dock_detials_id, dock_content_browser_id, dock_main_viewport_id, dock_scene_browser_id;
			ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.80f, &dock_content_browser_id, &dock_detials_id);
			ImGui::DockBuilderSplitNode(dock_content_browser_id, ImGuiDir_Down, 0.32f, &dock_content_browser_id, &dock_scene_browser_id);
			ImGui::DockBuilderSplitNode(dock_scene_browser_id, ImGuiDir_Right, 0.8f, &dock_main_viewport_id, &dock_scene_browser_id);

			// 将窗口分配到 dock 节点
			//ImGui::DockBuilderDockWindow(EditorText{ "Details" }.c_str(), dock_detials_id);
			//ImGui::DockBuilderDockWindow(EditorText{ "Content Browser",ICON_FA_FOLDERS }.c_str(), dock_content_browser_id);
			//ImGui::DockBuilderDockWindow(EditorText{ "Console", ICON_FA_TERMINAL }.c_str(), dock_content_browser_id);
			//ImGui::DockBuilderDockWindow(EditorText{ "Scene Browser",ICON_FA_LIST_UL }.c_str(), dock_scene_browser_id);
			//ImGui::DockBuilderDockWindow(EditorText{ "Main Viewport", ICON_FA_WINDOW }.c_str(), dock_main_viewport_id);

			ImGui::DockBuilderDockWindow(GetPanelWithID("DetailBrowserPanel")->GetWindowFullName().c_str(), dock_detials_id);
			ImGui::DockBuilderDockWindow(GetPanelWithID("ContentBrowserPanel")->GetWindowFullName().c_str(), dock_content_browser_id);
			ImGui::DockBuilderDockWindow(GetPanelWithID("ConsolePanel")->GetWindowFullName().c_str(), dock_content_browser_id);
			ImGui::DockBuilderDockWindow(GetPanelWithID("SceneBrowserPanel")->GetWindowFullName().c_str(), dock_scene_browser_id);
			ImGui::DockBuilderDockWindow(GetPanelWithID("EditorViewport")->GetWindowFullName().c_str(), dock_main_viewport_id);

			// 完成布局
			ImGui::DockBuilderFinish(dockSpaceID);
			//ImGui::SetNextWindowSize(viewport->Size / 2);

			m_DockspaceLayoutInitialized = true;

			// 重新布局设置完成
			m_DockspaceLayoutRequestRearrange = false;
		}
	}

	void EditorMainWindow::TraversePanels(std::function<void(IEditorPanel* panel)> callback)
	{
		for (auto& pair : m_IDPanels)
		{
			auto& panel = pair.second;

			if (panel)
			{
				callback(panel.get());
			}
			
		}
	}

	IEditorPanel* EditorMainWindow::GetPanelWithID(const std::string& id)
	{
		auto result = m_IDPanels.find(id);
		if (result != m_IDPanels.end())
		{
			return result->second.get();
		}

		return nullptr;
	}
}
