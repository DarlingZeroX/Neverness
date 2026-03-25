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

#include "EditorCore/EditorPreferences.h"
#include "PanelManager.h"
#include "Setting/Style.h"
#include "EditorCore/Localization.h"
#include "HFileSystem/Interface/HFileSystem.h"
#include "MainEditor/MainPanel.h"
#include "VGCore/Include/Core/VFS.h"
#include "VGImgui/IncludeImGuiEx.h"

namespace VisionGal::Editor
{
	EditorPreferenceEditor::EditorPreferenceEditor()
	{
		EditorTheme = EditorStyle::GetEditorThemes()[0]; // 默认主题
	}

	void EditorPreferenceEditor::OnGUI()
	{
		ImGui::Text(EditorText{ "Editor" }.c_str());

		static ImGuiTableFlags flags = GetSettingTableFlag();

		if (ImGui::BeginTable("EditorPreferenceEditor", 2, flags))
		{
			// 编辑器名称
			DrawTableColumnTitle("Editor Name");
			ImGuiEx::InputText("##Editor Name", EditorName);

			// 编辑器窗口宽度
			DrawTableColumnTitle("Editor Window Width");
			ImGui::InputInt("##Editor Window Width", &EditorWindowWidth);

			// 编辑器窗口高度
			DrawTableColumnTitle("Editor Window Height");
			ImGui::InputInt("##Editor Window Height", &EditorWindowHeight);

			// 编辑器语言
			DrawTableColumnTitle("Editor Language");
			if (ImGui::BeginCombo("##Editor Language", EditorText{ EditorLanguage }.c_str(), ImGuiComboFlags_PopupAlignLeft))
			{
				const auto& languageTypeCode = Horizon::HLocalizationManager::GetInstance()->GetLanguageTypeCode(); // 默认主题

				for (int n = 0; n < languageTypeCode.size(); n++)
				{
					if (ImGui::Selectable(EditorText{ languageTypeCode[n] }.c_str()))
					{
						EditorLanguage = languageTypeCode[n];
						Horizon::HLocalizationManager::GetInstance()->SetLanguageByCode(EditorLanguage);

						// 设置完语言需要重新布局编辑器UI
						auto* panelManager = PanelManager::GetInstance();
						auto* mainWindow = dynamic_cast<EditorMainWindow*>(panelManager->GetPanelWithID("EditorMainWindow"));
						mainWindow->RequestRearrangeLayout();
						Horizon::HFileSystem::RemoveFile("imgui.ini"); // 删除布局文件，强制重新布局
					}
				}
				ImGui::EndCombo();
			}

			// 编辑器主题
			DrawTableColumnTitle("Editor Theme");
			if (ImGui::BeginCombo("##Editor Theme", EditorText{ EditorTheme }.c_str(), ImGuiComboFlags_PopupAlignLeft))
			{
				const auto& themes = EditorStyle::GetEditorThemes(); // 默认主题

				for (int n = 0; n < themes.size(); n++)
				{
					if (ImGui::Selectable(EditorText{ themes[n] }.c_str()))
					{
						EditorTheme = themes[n];
						EditorStyle::SetTheme(EditorTheme);
					}
				}
				ImGui::EndCombo();
			}

			// 编辑器字体大小
			DrawTableColumnTitle("Editor Font Size");
			if (ImGui::InputInt("##Editor Font Size", &EditorFontSize))
			{
				EnsureDataNormal();
				ImGui::GetStyle().FontSizeBase = static_cast<float>(EditorFontSize);
				ImGui::GetStyle()._NextFrameFontSizeBase = ImGui::GetStyle().FontSizeBase; // 更新字体大小
			}

			EnsureDataNormal();

			ImGui::EndTable();
		}
	}

	void EditorPreferenceEditor::Load(const nlohmann::json& json)
	{
		EditorName = json.value("EditorName", EditorName);
		EditorWindowWidth = json.value("EditorWindowWidth", EditorWindowWidth);
		EditorWindowHeight = json.value("EditorWindowHeight", EditorWindowHeight);
		EditorTheme = json.value("EditorTheme", EditorTheme);
		EditorFontSize = json.value("EditorFontSize", EditorFontSize);
		EditorLanguage = json.value("EditorLanguage", EditorLanguage);

		if (EditorName.empty())
		{
			EditorName = "VisionGal Editor"; // 默认编辑器名称
		}

		EnsureDataNormal();
	}

	void EditorPreferenceEditor::Save(nlohmann::json& json)
	{
		EnsureDataNormal();

		json["EditorName"] = EditorName;
		json["EditorWindowWidth"] = EditorWindowWidth;
		json["EditorWindowHeight"] = EditorWindowHeight;
		json["EditorTheme"] = EditorTheme;
		json["EditorFontSize"] = EditorFontSize;
		json["EditorLanguage"] = EditorLanguage;
	}

	void EditorPreferenceEditor::EnsureDataNormal()
	{
		EditorWindowWidth = std::max(EditorWindowWidth, 10); // 确保宽度不小于10
		EditorWindowWidth = std::min(EditorWindowWidth, 100000);

		EditorWindowHeight = std::max(EditorWindowHeight, 10); // 确保高度不小于10
		EditorWindowHeight = std::min(EditorWindowHeight, 100000);

		EditorFontSize = std::max(EditorFontSize, 2);
		EditorFontSize = std::min(EditorFontSize, 60);
	}

	EditorPreferences::EditorPreferences()
	{
		m_PreferencesNameList = {
			"Editor"
		};
	}

	EditorPreferences::~EditorPreferences()
	{
	}

	void EditorPreferences::Load(EditorPreferences& preferences)
	{
		if (Horizon::HFileSystem::ExistsDirectory("Data") == false)
			return;

		if (Horizon::HFileSystem::ExistsDirectory("Data/Preferences") == false)
			return;

		ReadSettingFromFile("Data/Preferences/Editor.setting", preferences.Editor);
	}

	void EditorPreferences::Save(EditorPreferences& preferences)
	{
		Horizon::HFileSystem::CreateDirectoryWhenNoExist("Data");
		Horizon::HFileSystem::CreateDirectoryWhenNoExist("Data/Preferences");

		SaveSettingToFile("Data/Preferences/Editor.setting", preferences.Editor);
	}

	std::string EditorPreferences::GetPreferencesNameByIndex(int index) const
	{
		if (index < 0 || index >= static_cast<int>(m_PreferencesNameList.size()))
			return "";

		return m_PreferencesNameList[index];
	}

	EditorSettingInterface* EditorPreferences::GetPreferencesByName(const std::string& name)
	{
		if (name == "Editor")
			return &Editor;
		return nullptr;
	}

	EditorSettingInterface* EditorPreferences::GetPreferencesByIndex(int index)
	{
		if (index < 0 || index >= static_cast<int>(m_PreferencesNameList.size()))
			return nullptr;

		return GetPreferencesByName(GetPreferencesNameByIndex(index));
	}

	void EditorPreferences::ReadSettingFromFile(const std::filesystem::path& path, EditorSettingInterface& setting)
	{
		std::string text;
		if (Horizon::HFileSystem::ReadTextFromFile(path, text))
		{
			try {
				nlohmann::json json = nlohmann::json::parse(text);
				setting.Load(json);
				H_LOG_INFO("The preferences configuration was parsed successfully: %s", path.string().c_str());
			}
			catch (const nlohmann::json::parse_error& e) {
				H_LOG_WARN("Error in parsing %s preferences configuration: %s", path.string().c_str(), e.what());
			}
		}
	}

	void EditorPreferences::SaveSettingToFile(const std::filesystem::path& path, EditorSettingInterface& setting)
	{
		nlohmann::json json;
		setting.Save(json);
		std::string jsonStr = json.dump(2);

		Horizon::HFileSystem::WriteTextToFile(path, jsonStr);
	}
}
