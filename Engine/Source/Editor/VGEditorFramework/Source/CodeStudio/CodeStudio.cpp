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

#include "CodeStudio/CodeStudio.h"
#include "VGEditorFramework/Include/EditorCore/Localization.h"
#include "HFileSystem/Interface/HFileSystem.h"
#include "VGAsset/Interface/Package.h"
#include "VGCore/Include/Core/EventBus.h"
#include "VGCore/Include/Core/VFS.h"
#include "VGImgui/Include/ImGuiEx/IconFont/IconsFontAwesome5Pro.h"

namespace VisionGal::Editor
{
	CodeStudioPanel::CodeStudioPanel()
	{
		EngineEventBus::Get().OnLuaScriptEvent.Subscribe([this](const LuaScriptEvent& evt)
			{
				// 处理脚本错误事件
				switch (evt.EventType)
				{
				case LuaScriptEventType::ScriptError:
					OpenWindow(true);
					OpenTextFile(evt.ScriptPath);

					if (auto* doc = m_DocManager.GetDocument(evt.ScriptPath))
					{
						ImGuiTextEditor::ErrorMarkers markers;
						markers[evt.ErrorLineNumber] = evt.ErrorMessage;
						doc->TexEditor.SetErrorMarkers(markers);
					}
					break;
				}
			});
	}

	CodeStudioPanel::~CodeStudioPanel()
	{
	}

	void CodeStudioPanel::OnGUI()
	{
		if (!m_IsOpen)
			return;

		ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiCond_FirstUseEver);

		if (ImGui::Begin(GetWindowFullName().c_str(), &m_IsOpen))
		{
			// 左侧
			//ImGui::BeginChild("CodeStudioPanel Left", ImVec2(200, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
			//RenderFileListUI();
			//ImGui::EndChild();
			//
			//ImGui::SameLine();
			//
			//// 右侧
			//ImGui::BeginChild("CodeStudioPanel Right", ImVec2(0, 0), ImGuiChildFlags_Borders);
			RenderTextEditorUI();
			//ImGui::EndChild();

			if (m_IsOpen == false)
			{
				m_DocManager.SaveAllDocument();
			}
		}

		ImGui::End();
	}

	void CodeStudioPanel::RenderTextEditorUI()
	{
		// Create a DockSpace node where any window can be docked
		ImGuiID dockspace_id = ImGui::GetID("CodeStudio");
		ImGui::DockSpace(dockspace_id);

		bool redock_all = false;

		// Create Windows
		for (auto& doc:  m_DocManager.Documents)
		{
			doc->Update();

			if (!doc->IsOpen)
				continue;

			ImGui::SetNextWindowDockID(dockspace_id, redock_all ? ImGuiCond_Always : ImGuiCond_FirstUseEver);
			ImGuiWindowFlags window_flags = (doc->Dirty ? ImGuiWindowFlags_UnsavedDocument : 0);
			bool visible = ImGui::Begin(doc->Name.c_str(), &doc->IsOpen, window_flags);

			// 窗口获得焦点
			if (doc->NeedFocus)
			{
				ImGui::SetWindowFocus();
				//ImGui::SetKeyboardFocusHere();
				if (ImGui::IsWindowFocused())
				{
					doc->NeedFocus = false;
				}
			}

			// Cancel attempt to close when unsaved add to save queue so we can display a popup.
			if (!doc->IsOpen && doc->Dirty)
			{
				doc->IsOpen = true;
				m_DocManager.CloseQueue.push_back(doc.get());
			}

			// 显示路径
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::Text(doc->DocPath.c_str());
				ImGui::EndTooltip();
			}

			DisplayDocContextMenu(doc.get());
			//app.DisplayDocContextMenu(doc);
			if (visible)
			{
				doc->TexEditor.Render("Editor");
				//app.DisplayDocContents(doc);
			}

			ImGui::End();
		}

		DisplayClosingConfirmationUI();
	}

	void CodeStudioPanel::DisplayClosingConfirmationUI()
	{
		// Display closing confirmation UI
		if (!m_DocManager.CloseQueue.empty())
		{
			int close_queue_unsaved_documents = 0;
			for (int n = 0; n < m_DocManager.CloseQueue.size(); n++)
				if (m_DocManager.CloseQueue[n]->Dirty)
					close_queue_unsaved_documents++;

			if (close_queue_unsaved_documents == 0)
			{
				// Close documents when all are unsaved
				for (int n = 0; n < m_DocManager.CloseQueue.size(); n++)
					m_DocManager.CloseQueue[n]->DoForceClose();
				m_DocManager.CloseQueue.clear();
			}
			else
			{
				if (!ImGui::IsPopupOpen("Save?"))
					ImGui::OpenPopup("Save?");

				// 确保弹出窗口获得焦点，防止被代码编辑器窗口覆盖
				ImGui::SetNextWindowFocus(); 
				if (ImGui::BeginPopupModal("Save?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
				{
					ImGui::Text(EditorText{ "Save change to the following files?"}.c_str());
					float item_height = ImGui::GetTextLineHeightWithSpacing();
					if (ImGui::BeginChild(ImGui::GetID("frame"), ImVec2(-FLT_MIN, 6.25f * item_height), ImGuiChildFlags_FrameStyle))
						for (auto* doc : m_DocManager.CloseQueue)
							if (doc->Dirty)
								ImGui::Text("%s", doc->DocPath.c_str());
					ImGui::EndChild();

					ImVec2 button_size(ImGui::GetFontSize() * 7.0f, 0.0f);
					if (ImGui::Button(EditorText{ "Save"}.c_str(), button_size))
					{
						for (auto* doc : m_DocManager.CloseQueue)
						{
							if (doc->Dirty)
								m_DocManager.SaveDocument(doc);
							m_DocManager.CloseDocument(doc);
						}
						m_DocManager.CloseQueue.clear();
						ImGui::CloseCurrentPopup();
					}
					ImGui::SameLine();
					if (ImGui::Button(EditorText{ "Don't Save" }.c_str(), button_size))
					{
						for (auto* doc : m_DocManager.CloseQueue)
							m_DocManager.CloseDocument(doc);
						m_DocManager.CloseQueue.clear();
						ImGui::CloseCurrentPopup();
					}
					ImGui::SameLine();
					if (ImGui::Button(EditorText{ "Cancel" }.c_str(), button_size))
					{
						m_DocManager.CloseQueue.clear();
						ImGui::CloseCurrentPopup();
					}
					ImGui::EndPopup();
				}
			}
		}
	}

	void CodeStudioPanel::RenderFileListUI()
	{
		for (auto& file: m_DocManager.FileList)
		{
			auto fileName = ICON_FA_FILE" " + file;
			ImGui::Selectable(fileName.c_str());
			
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				OpenTextFile(file);
			}
		}
	}

	bool CodeStudioPanel::OpenTextFile(const VGPath& path)
	{
		return m_DocManager.OpenTextFile(path);
	}

	std::string CodeStudioPanel::GetWindowFullName()
	{
		return EditorText{ GetWindowName() }.GetText();
	}

	std::string CodeStudioPanel::GetWindowName()
	{
		return "Code Studio";
	}

	void CodeStudioPanel::OpenWindow(bool open)
	{
		m_IsOpen = open;
	}

	bool CodeStudioPanel::IsWindowOpened()
	{
		return m_IsOpen;
	}

	void CodeStudioPanel::DisplayDocContextMenu(CodeDocument* doc)
	{
		if (!ImGui::BeginPopupContextItem())
			return;

		if (ImGui::MenuItem(EditorText{ "Save"}.c_str(), "Ctrl+S", false, doc->IsOpen))
			doc->DoSave();
		if (ImGui::MenuItem(EditorText{ "Close"}.c_str(), "", false, doc->IsOpen))
			m_DocManager.CloseQueue.push_back(doc);
		ImGui::EndPopup();
	}
}
