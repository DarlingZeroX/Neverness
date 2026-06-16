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
#include "ProjectSettingInterface.h"
#include "../EngineConfig.h"

namespace NN::Runtime
{
	struct VG_ENGINE_API ProjectSettingsEditor : public ProjectSettingInterface
	{
		std::string MainScene;

		void Load(const nlohmann::json& json) override;
		void Save(nlohmann::json& json) override;

		void EnsureDataNormal();
	};

	struct VG_ENGINE_API ProjectSettingsApplication : public ProjectSettingInterface
	{
		std::string ApplicationName = "GalGame";
		std::string RunningMainScene;
		int WindowWidth = 1280;
		int WindowHeight = 720;
		bool WindowAllowResize = true;

		void Load(const nlohmann::json& json) override;
		void Save(nlohmann::json& json) override;

		void EnsureDataNormal();
	};

	struct VG_ENGINE_API ProjectSettingsGalGame : public ProjectSettingInterface
	{
		int DesignWidth = 1920;
		int DesignHeight = 1080;

		void Load(const nlohmann::json& json) override;
		void Save(nlohmann::json& json) override;

		void EnsureDataNormal();
	};

	struct VG_ENGINE_API ProjectSettings
	{
		ProjectSettingsEditor Editor;
		ProjectSettingsApplication Application;
		ProjectSettingsGalGame GalGame;

		ProjectSettings(); 

		const std::vector<std::string>& GetSettingsNameList() const { return m_SettingsNameList; }
		std::string GetSettingsNameByIndex(int index) const;
		ProjectSettingInterface* GetSettingsByName(const std::string& name);
		ProjectSettingInterface* GetSettingsByIndex(int index);

		static void Load(ProjectSettings& settings);
		static void Save(ProjectSettings& settings);

		static void LoadProjectSettings();
		static void SaveProjectSettings();
		static ProjectSettings& GetProjectSettings();

		static void ReadSettingFromFileVFS(const std::string& pathVFS, ProjectSettingInterface& setting);
		static void SaveSettingToFileVFS(const std::string& pathVFS, ProjectSettingInterface& setting);
	private:
		std::vector<std::string> m_SettingsNameList;
	};

}
