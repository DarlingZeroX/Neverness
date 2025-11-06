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

#include "EditorComponents/TextEditor.h"
#include <fstream>
#include <sstream>

#include "EditorCore/Localization.h"
#include "VGEngine/Include/Core/VFS.h"
//#include "VGImgui/Include/ImGuiColorTextEdit/TextEditorOriginal.h"

namespace VisionGal::Editor
{
	extern const std::filesystem::path g_ShadersPath = "HLSL";

	class TextEditorPanel::ShaderEditorList : public ImGuiEx::ImWindow
	{
	public:
		ShaderEditorList(TextEditorPanel* __This)
			:ImWindow("Shader List"), This(__This)
		{
		}
	protected:
		void OnWindowGUI() override
		{
			DrawDirectoryTree(g_ShadersPath);
		}

		void DrawDirectoryTree(const std::filesystem::path& path)
		{
			for (auto& directoryEntry : std::filesystem::directory_iterator(path))
			{
				if (directoryEntry.is_directory())
				{
					const auto& path = directoryEntry.path();
					const auto filenameString = std::filesystem::relative(path, g_ShadersPath).filename().string();
					std::string nodeName = ICON_FA_FOLDER " " + filenameString;

					if (ImGui::TreeNodeEx(nodeName.c_str(), ImGuiTreeNodeFlags_SpanFullWidth))
					{
						DrawDirectoryTree(path);
						ImGui::TreePop();
					}
				}
			}

			const bool mouseClicked = ImGui::IsMouseClicked(0);
			for (auto& directoryEntry : std::filesystem::directory_iterator(path))
			{
				if (!directoryEntry.is_directory())
				{
					const auto& path = directoryEntry.path();
					const auto filenameString = std::filesystem::relative(path, g_ShadersPath).filename().string();
					std::string nodeName = ICON_FA_FILE_CODE " " + filenameString;

					if (ImGui::TreeNodeEx(nodeName.c_str(), ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_Leaf))
					{
						ImGui::TreePop();
					}

					if (mouseClicked && ImGui::IsItemHovered())
					{
						//This->OpenTextFile(path);
					}
				}
			}
		}

	private:
		TextEditorPanel* This;
	};

	TextEditorPanel::TextEditorPanel()
		:m_ListPanel(new ShaderEditorList(this))
	{
		using ImGuiTextEditor::TextEditor;

		//SetWindowName("Shader Editor");

		auto lang = ImGuiTextEditor::LanguageDefinition::GalGameScript();
		m_TexEditor.SetLanguageDefinition(lang);
		m_TexEditor.SetPalette(TextEditor::GetDarkPalette());
		m_TexEditor.SetActiveAutocomplete(true);

		//auto lang = ImGuiTextEditorOriginal::TextEditor::LanguageDefinition::Lua();
		//m_TexEditor.SetLanguageDefinition(lang);
		//m_TexEditor.SetPalette(ImGuiTextEditorOriginal::TextEditor::GetDarkPalette());
	}

	TextEditorPanel::~TextEditorPanel()
	{
	}

	void TextEditorPanel::OnGUI()
	{
		if (!m_IsOpen)
			return;

		ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiCond_FirstUseEver);

		if (ImGui::Begin(GetWindowFullName().c_str(), &m_IsOpen))
		{
			// 左侧
			ImGui::BeginChild("TextEditorPanel Left", ImVec2(200, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
			RenderFileListUI();
			ImGui::EndChild();

			ImGui::SameLine();

			// 右侧
			ImGui::BeginChild("TextEditorPanel Right", ImVec2(0, 0), ImGuiChildFlags_Borders);
			RenderTextEditorUI();
			ImGui::EndChild();
		}

		ImGui::End();
	}

	void TextEditorPanel::RenderTextEditorUI()
	{
		if (m_CurrentTextPath.empty())
			return;

		if (m_TexEditor.IsTextChanged())
		{
			m_IsTextChanged = true;
		}

		auto windowName = EditorText{ "Text Editor" }.GetText() + ": " + m_CurrentTextPath;
		{
			//static std::string test;
			//ImGuiEx::InputText("Steawste", test);
			if (m_HasText)
			{

				//if (m_TexEditor.IsEditorHovered())
				//{
				ImGuiViewport* imViewport = ImGui::GetWindowViewport();
				SDL_WindowID windowID = reinterpret_cast<SDL_WindowID>(imViewport->PlatformHandle);
				SDL_StartTextInput(SDL_GetWindowFromID(windowID));
				//H_LOG_INFO("SDL_StartTextInput");
			//}
			//else
			//{
			//	//H_LOG_INFO("resize");
			//	ImGuiEx::GetIO().InputQueueCharacters.resize(0);
			//}

				m_TexEditor.Render("Editor");
			}

		}
	}

	void TextEditorPanel::RenderFileListUI()
	{
		for (auto& file: m_FileList)
		{
			auto fileName = ICON_FA_FILE" " + file;
			ImGui::Selectable(fileName.c_str());
			
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				OpenTextFile(file);
			}
		}
	}

	bool TextEditorPanel::OpenTextFile(const VGPath& path)
	{
		auto result = VFS::ReadTextFromFile(path, m_Text);

		if (result == false)
			return false;

		m_TexEditor.SetText(m_Text);
		m_HasText = true;
		m_CurrentTextPath = path;
		m_FileList.insert(path);

		return true;
	}

	std::string TextEditorPanel::GetWindowFullName()
	{
		return EditorText{ GetWindowName() }.GetText();
	}

	std::string TextEditorPanel::GetWindowName()
	{
		return "TextEditor";
	}

	void TextEditorPanel::OpenWindow(bool open)
	{
		m_IsOpen = open;
	}

	bool TextEditorPanel::IsWindowOpened()
	{
		return m_IsOpen;
	}

}
