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

#include "MainEditor/MenuBar.h"
//#include "VGImgui/Include/ImGuiEx/IconFont/IconsFontAwesome5Pro.h"
#include <VGImgui/IncludeImGuiEx.h>
#include "PanelManager.h"
#include "EditorCore/EditorCore.h"
#include "EditorCore/Localization.h"
#include "EditorCore/EdtiorScene.h"
#include "HCore/Include/System/HSystemMisc.h"
#include "MainEditor/MainPanel.h"
#include "VGEngine/Include/Core/Input.h"
#include "VGEngine/Include/Engine/Manager.h"
#include "VGEngine/Include/Engine/VGEngine.h"
#include "VGEngine/Include/Interface/Loader.h"

namespace VisionGal::Editor
{
	EditorMenuBar::EditorMenuBar(VGWindow* window)
		:m_EditorWindow(window)
	{
		m_EngineIcon = LoadObject<Texture2D>(  "/editor/icons/engineIcon.png");
	}

	void EditorMenuBar::OnGUI()
	{
		static bool s_ShowImguiStyleEditor = false;
		auto& style = ImGui::GetStyle();
		auto borderSize = style.WindowBorderSize;
		style.WindowBorderSize = 0.0f;

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 7));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 4));

		if (ImGui::BeginMainMenuBar())
		{
			// 拖动窗口
			HandleDraggingWindow();

			// 引擎图标
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
			ImVec2 size = { 40,ImGui::GetFrameHeight() * 2 };
			ImGui::Image(m_EngineIcon->GetTexture()->GetShaderResourceView(),
				size);
			ImGui::PopStyleVar();

			// 文件菜单
			if (ImGui::BeginMenu(EditorText{ "File" }.c_str()))
			{
				ImGui::Separator();
				if (ImGui::MenuItemEx(EditorText{ "New Scene" }.c_str(), " ", "CTRL+N"))
				{
					EditorScene::NewScene();
				}
				if (ImGui::MenuItemEx(EditorText{ "Open Scene" }.c_str(), ICON_FA_FILE, "CTRL+O"))
				{
					EditorScene::OpenSceneByFileDialog();
				}

				ImGui::Separator();
				if (ImGui::MenuItemEx(EditorText{ "Save Scene" }.c_str(), ICON_FA_SAVE, "CTRL+S"))
				{
					EditorScene::SaveCurrentScene();
				}

				if (ImGui::MenuItemEx(EditorText{ "Save Scene As..." }.c_str(), ICON_FA_SAVE))
				{
					EditorScene::SaveCurrentSceneAs();
				}

				ImGui::Separator();

				//if (ImGui::MenuItem(EditorText{ "New Project" }.c_str(), ""))
				//{
				//}
				//if (ImGui::MenuItem(EditorText{ "Open Project" }.c_str(), ""))
				//{
				//}
				//if (ImGui::MenuItemEx(EditorText{ "Save Project" }.c_str(), ICON_FA_SAVE, ""))
				//{
				//}

				if (ImGui::MenuItem(EditorText{ "Build Setting..." }.c_str()))
				{
					PanelManager::GetInstance()->OpenPanel("BuildSettings");
				}

				ImGui::Separator();

				if (ImGui::MenuItemEx(EditorText{ "Exit" }.c_str(), ICON_FA_TIMES))
				{
					VGEngine::Get()->RequestExit();
				}
				ImGui::EndMenu();
			}

			// 编辑
			if (ImGui::BeginMenu(EditorText{ "Edit" }.c_str()))
			{
				//if (ImGui::MenuItemEx("Undo", " ", "CTRL+Z")) {}
				//if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
				//ImGui::Separator();
				//if (ImGui::MenuItem("Cut", "CTRL+X")) {}
				//if (ImGui::MenuItem("Copy", "CTRL+C")) {}
				//if (ImGui::MenuItem("Paste", "CTRL+V")) {}

				if (ImGui::MenuItem(EditorText{ "Imgui Demo" }.c_str()))
				{
					s_ShowImguiStyleEditor = true;
				}

				if (ImGui::MenuItem(EditorText{ "Project Setting..." }.c_str()))
				{
					PanelManager::GetInstance()->OpenPanel("ProjectSetting");
				}

				if (ImGui::MenuItem(EditorText{ "Preferences..." }.c_str()))
				{
					PanelManager::GetInstance()->OpenPanel("EditorPreferences");
				}
				ImGui::EndMenu();
			}

			// 窗口菜单
			if (ImGui::BeginMenu(EditorText{ "Window" }.c_str()))
			{
				auto* panelManager = PanelManager::GetInstance();
				auto* mainWindow = dynamic_cast<EditorMainWindow*>(panelManager->GetPanelWithID("EditorMainWindow"));
				if (mainWindow)
				{
					mainWindow->TraversePanels([this](IEditorPanel* panel)
						{
							if (panel == nullptr)
								return;
							std::string icon;
							if (panel->IsWindowOpened())
								icon = ICON_FA_CHECK;
							else
								icon = ICON_FA_TIMES;

							if (ImGui::MenuItemEx(EditorText{ panel->GetWindowName() }.c_str(), icon.c_str()))
							{
								panel->OpenWindow(!panel->IsWindowOpened());
							}
						});
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu(EditorText{ "Help" }.c_str()))
			{
				if (ImGui::MenuItem(EditorText{ "Engine homepage" }.c_str()))
				{
					Horizon::HSystemMisc::OpenURL("https://darlingzerox.github.io/VisionGalDoc/");
				}

				if (ImGui::MenuItem(EditorText{ "GitHub" }.c_str()))
				{
					Horizon::HSystemMisc::OpenURL("https://github.com/DarlingZeroX/VisionGal");
				}

				ImGui::EndMenu();
			}

			HandleWindowControl();
			ImGui::EndMainMenuBar();
		}

		ImGui::PopStyleVar();
		ImGui::PopStyleVar();

		style.WindowBorderSize = borderSize;

		if (s_ShowImguiStyleEditor)
		{
			ImGui::ShowDemoWindow(&s_ShowImguiStyleEditor);
		}
	}

	std::string EditorMenuBar::GetWindowFullName()
	{
		return "EditorMenuBar";
	}

	std::string EditorMenuBar::GetWindowName()
	{
		return "EditorMenuBar";
	}

	void EditorMenuBar::OpenWindow(bool open)
	{
	}

	bool EditorMenuBar::IsWindowOpened()
	{
		return true;
	}

	void EditorMenuBar::HandleDraggingWindow()
	{
		if (!ImGui::IsWindowHovered() && !m_bDragging)
			return;

		if (!ImGui::IsMouseDragging(ImGuiMouseButton_Left))
		{
			m_bDragging = false;
			return;
		}

		static int2 position = int2(0, 0);
		if (m_bDragging == false)
		{
			position = m_EditorWindow->GetWindowPos();
		}

		m_bDragging = true;
		if (m_EditorWindow && m_EditorMaximized == false)
		{
			auto dragDelta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
			//std::cout << "Drag Delta: " << dragDelta.x << ", " << dragDelta.y << std::endl;
			//std::cout << "position: " << position.x << ", " << position.y << std::endl;
			int x = position.x + dragDelta.x;
			int y = position.y + dragDelta.y;
			y = std::max(y, 0); // 不允许拖出屏幕顶部
			m_EditorWindow->SetWindowPos(x, y);
		}
	}

	void EditorMenuBar::HandleWindowControl()
	{
		if (m_EditorWindow == nullptr)
			return;

		//ImGui::SameLine(ImGui::GetWindowWidth() - 160);
		ImGui::Indent(ImGui::GetWindowWidth() - 160);

		ImGui::PushID("MainMenuBarPanel WindowControl");
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.1f, 0.1f, 0.1f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 1.0f, 1.0f, 1.0f, 0.2f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 1.0f, 1.0f, 1.0f, 0.4f });
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4{ 0.0f, 0.0f, 0.0f, 0.01f });

		ImVec2 size = { 40,ImGui::GetFrameHeight() };

		if (ImGui::Button(ICON_FA_MINUS"##WindowMin", size))
		{
			m_EditorWindow->MinimizeWindow();
		}//ImGui::SameLine();

		static int2 windowSize;
		static int2 windowPos;
		if (m_EditorMaximized)
		{
			if (ImGui::Button(ICON_FA_WINDOW_RESTORE"##WindowMax", size))
			{
				//m_EditorWindow->RestoreWindow();
				m_EditorWindow->SetWindowPos(windowPos.x, windowPos.y);
				m_EditorWindow->SetWindowSize(windowSize.x, windowSize.y);
				m_EditorMaximized = false;
			}//ImGui::SameLine();
		}
		else
		{
			if (ImGui::Button(ICON_FA_WINDOW_MAXIMIZE"##WindowMax", size))
			{
				windowPos = m_EditorWindow->GetWindowPos();
				windowSize = m_EditorWindow->GetWindowSize();
				//m_EditorWindow->MaximizeWindow();
				SDL_Rect usable;
				SDL_GetDisplayUsableBounds(SDL_GetPrimaryDisplay(), &usable);
				m_EditorWindow->SetWindowPos(usable.x, usable.y);
				m_EditorWindow->SetWindowSize(usable.w, usable.h);
				m_EditorMaximized = true;
			}//ImGui::SameLine();
		}


		if (ImGui::Button(ICON_FA_TIMES"##WindowClose", size))
		{
			VGEngine::Get()->RequestExit();
		}

		ImGui::PopStyleColor(4);
		ImGui::PopID();
	}
}
