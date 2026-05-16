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

#include "Setting/ProjectSettingsWrapper.h"
#include "EditorCore/Localization.h"
#include "NNRuntimeCore/Include/Core/Core.h"
#include "NNEngineLegacy/Include/Project/ProjectSettings.h"
#include "NNRuntimeImGui/IncludeImGuiEx.h"

namespace NN::Editor
{
	void ProjectSettingsEditorWrapper::OnGUI()
	{
		auto& setting = Runtime::ProjectSettings::GetProjectSettings().Editor;

		ImGui::Text(EditorText{ "Editor" }.c_str());

		static ImGuiTableFlags flags = GetSettingTableFlag();

		if (ImGui::BeginTable("ProjectSettingsEditor", 2, flags))
		{
			// 编辑器启动的主场景
			DrawTableColumnTitle("Editor Main Scene");
			ImGuiEx::InputText("##Editor Name", setting.MainScene, ImGuiInputTextFlags_ReadOnly);

			setting.EnsureDataNormal();

			ImGui::EndTable();
		}
	}

	void ProjectSettingsApplicationWrapper::OnGUI()
	{
		auto& setting = Runtime::ProjectSettings::GetProjectSettings().Application;

		ImGui::Text(EditorText{ "Application" }.c_str());

		static ImGuiTableFlags flags = GetSettingTableFlag();

		if (ImGui::BeginTable("ProjectSettingsApplication", 2, flags))
		{
			// 应用程序名称
			DrawTableColumnTitle("Application Name");
			ImGuiEx::InputText("##Application Name", setting.ApplicationName);

			// 应用程序的主场景
			DrawTableColumnTitle("Application Main Scene");
			ImGuiEx::InputText("##Application Main Scene", setting.RunningMainScene);

			// 应用程序的窗口宽度
			DrawTableColumnTitle("Window Width");
			ImGui::InputInt("##Application Window Width", &setting.WindowWidth);

			// 应用程序的窗口高度
			DrawTableColumnTitle("Window Height");
			ImGui::InputInt("##Application Window Height", &setting.WindowHeight);

			// 应用程序运行是否允许窗口大小调整
			DrawTableColumnTitle("Window Allow Resize");
			ImGui::Checkbox("##Application Window Allow Resize", &setting.WindowAllowResize);

			setting.EnsureDataNormal();

			ImGui::EndTable();
		}
	}

	void ProjectSettingsGalGameWrapper::OnGUI()
	{
		auto& setting = Runtime::ProjectSettings::GetProjectSettings().GalGame;

		ImGui::Text(EditorText{ "GalGame" }.c_str());

		static ImGuiTableFlags flags = GetSettingTableFlag();

		if (ImGui::BeginTable("ProjectSettingsGalGame", 2, flags))
		{
			// GalGame的设计宽度
			DrawTableColumnTitle("Design Width");
			ImGui::InputInt("##GalGame Design Width", &setting.DesignWidth);

			// GalGame的设计高度
			DrawTableColumnTitle("Design Height");
			ImGui::InputInt("##GalGame Design Height", &setting.DesignHeight);

			setting.EnsureDataNormal();

			ImGui::EndTable();
		}
	}

	ProjectSettingsWrapper::ProjectSettingsWrapper()
	{
	}

	ProjectSettingsWrapper::~ProjectSettingsWrapper()
	{
	}

	const std::vector<std::string>& ProjectSettingsWrapper::GetProjectSettingsNameList()
	{
		return Runtime::ProjectSettings::GetProjectSettings().GetSettingsNameList();
	}

	std::string ProjectSettingsWrapper::GetProjectSettingsNameByIndex(int index)
	{
		return Runtime::ProjectSettings::GetProjectSettings().GetSettingsNameByIndex(index);
	}

	EditorSettingInterface* ProjectSettingsWrapper::GetProjectSettingsByName(const std::string& name)
	{
		static ProjectSettingsEditorWrapper EditorWrapper;
		static ProjectSettingsApplicationWrapper ApplicationWrapper;
		static ProjectSettingsGalGameWrapper GalGameWrapper;

		if (name == "Editor")
			return &EditorWrapper;

		if (name == "Application")
			return &ApplicationWrapper;

		if (name == "GalGame")
			return &GalGameWrapper;

		return nullptr;
	}

	EditorSettingInterface* ProjectSettingsWrapper::GetProjectSettingsByIndex(int index)
	{
		return GetProjectSettingsByName(GetProjectSettingsNameByIndex(index));
	}
}
