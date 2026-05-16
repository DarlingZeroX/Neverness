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

#include "Project/ProjectSettings.h"
#include "NNRuntimeCore/Include/Core/VFS.h"
#include "Engine/Manager.h"

namespace VisionGal
{
	struct ProjectSettingsImp
	{
		ProjectSettingsImp()
		{

		}

		static ProjectSettingsImp* GetInstance()
		{
			static ProjectSettingsImp imp;
			return &imp;
		}

		void Initialize()
		{
		}

		ProjectSettings m_ProjectSettings;
	};

	void ProjectSettingsEditor::Load(const nlohmann::json& json)
	{
		MainScene = json.value("MainScene", MainScene);

		EnsureDataNormal();
	}

	void ProjectSettingsEditor::Save(nlohmann::json& json)
	{
		EnsureDataNormal();

		if (auto* editorScene = GetSceneManager()->GetCurrentEditorScene())
		{
			MainScene = editorScene->GetResourcePath();
		}

		json["MainScene"] = MainScene;
	}

	void ProjectSettingsEditor::EnsureDataNormal()
	{
	}

	void ProjectSettingsApplication::Load(const nlohmann::json& json)
	{
		ApplicationName = json.value("ApplicationName", ApplicationName);
		RunningMainScene = json.value("RunningMainScene", RunningMainScene);
		WindowWidth = json.value("WindowWidth", WindowWidth);
		WindowHeight = json.value("WindowHeight", WindowHeight);
		WindowAllowResize = json.value("WindowAllowResize", WindowAllowResize);

		EnsureDataNormal();
	}

	void ProjectSettingsApplication::Save(nlohmann::json& json)
	{
		EnsureDataNormal();

		json["ApplicationName"] = ApplicationName;
		json["RunningMainScene"] = RunningMainScene;
		json["WindowWidth"] = WindowWidth;
		json["WindowHeight"] = WindowHeight;
		json["WindowAllowResize"] = WindowAllowResize;
	}

	void ProjectSettingsApplication::EnsureDataNormal()
	{
		WindowWidth = std::max(WindowWidth, 10); // 确保宽度不小于10
		WindowWidth = std::min(WindowWidth, 100000);

		WindowHeight = std::max(WindowHeight, 10); // 确保高度不小于10
		WindowHeight = std::min(WindowHeight, 100000);
	}

	void ProjectSettingsGalGame::Load(const nlohmann::json& json)
	{
		DesignWidth = json.value("DesignWidth", DesignWidth);
		DesignHeight = json.value("DesignHeight", DesignHeight);

		EnsureDataNormal();
	}

	void ProjectSettingsGalGame::Save(nlohmann::json& json)
	{
		EnsureDataNormal();

		json["DesignWidth"] = DesignWidth;
		json["DesignHeight"] = DesignHeight;
	}

	void ProjectSettingsGalGame::EnsureDataNormal()
	{
		DesignWidth = std::max(DesignWidth, 10); // 确保宽度不小于10
		DesignWidth = std::min(DesignWidth, 100000);

		DesignHeight = std::max(DesignHeight, 10); // 确保高度不小于10
		DesignHeight = std::min(DesignHeight, 100000);
	}

	ProjectSettings::ProjectSettings()
	{
		m_SettingsNameList = {
			"Editor",
			"Application",
			"GalGame"
		};
	}

	std::string ProjectSettings::GetSettingsNameByIndex(int index) const
	{
		if (index < 0 || index >= static_cast<int>(m_SettingsNameList.size()))
			return "";

		return m_SettingsNameList[index];
	}

	ProjectSettingInterface* ProjectSettings::GetSettingsByName(const std::string& name)
	{
		if (name == "Editor")
			return &Editor;

		if (name == "Application")
			return &Application;

		if (name == "GalGame")
			return &GalGame;

		return nullptr;
	}

	ProjectSettingInterface* ProjectSettings::GetSettingsByIndex(int index)
	{
		if (index < 0 || index >= static_cast<int>(m_SettingsNameList.size()))
			return nullptr;

		return GetSettingsByName(GetSettingsNameByIndex(index));
	}

	void ProjectSettings::Load(ProjectSettings& settings)
	{
		ReadSettingFromFileVFS("Editor.setting", settings.Editor);
		ReadSettingFromFileVFS("Application.setting", settings.Application);
		ReadSettingFromFileVFS("GalGame.setting", settings.GalGame);
	}

	void ProjectSettings::Save(ProjectSettings& settings)
	{
		SaveSettingToFileVFS("Editor.setting", settings.Editor);
		SaveSettingToFileVFS("Application.setting", settings.Application);
		SaveSettingToFileVFS("GalGame.setting", settings.GalGame);
	}

	void ProjectSettings::LoadProjectSettings()
	{
		Load(ProjectSettingsImp::GetInstance()->m_ProjectSettings);
	}

	void ProjectSettings::SaveProjectSettings()
	{
		Save(ProjectSettingsImp::GetInstance()->m_ProjectSettings);
	}

	ProjectSettings& ProjectSettings::GetProjectSettings()
	{
		return ProjectSettingsImp::GetInstance()->m_ProjectSettings;
	}

	void ProjectSettings::ReadSettingFromFileVFS(const std::string& pathVFS, ProjectSettingInterface& setting)
	{
		std::string path = Core::GetProjectSettingsPathVFS();
		path = path + pathVFS;

		std::string text;
		if (VFS::ReadTextFromFile(path, text))
		{
			try {
				nlohmann::json json = nlohmann::json::parse(text);
				setting.Load(json);
				H_LOG_INFO("The preferences configuration was parsed successfully: %s", pathVFS.c_str());
			}
			catch (const nlohmann::json::parse_error& e) {
				H_LOG_WARN("Error in parsing %s preferences configuration: %s", pathVFS.c_str(), e.what());
			}
		}
	}

	void ProjectSettings::SaveSettingToFileVFS(const std::string& pathVFS, ProjectSettingInterface& setting)
	{
		nlohmann::json json;
		setting.Save(json);
		std::string jsonStr = json.dump(2);

		std::string path = Core::GetProjectSettingsPathVFS();
		path = path + pathVFS;

		VFS::WriteTextToFile(path, jsonStr);
	}
}
