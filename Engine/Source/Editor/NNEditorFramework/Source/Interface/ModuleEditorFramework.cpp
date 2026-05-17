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

#include "ModuleEditorFramework.h"
#include "AssetImporter/AssetImporterManager.h"
#include "EditorCore/ContentBrowser.h"
#include "NNRuntimeCore/Include/Core/VFS.h"
#include "../Framework.h"

namespace NN::Editor
{
	void ModuleEditorFramework::MountToEditor(Ref<Runtime::VGWindow>& editorWindow, Ref<Runtime::CoreGameEngine>& gameEngine)
	{
		// 初始化内容浏览器
		Runtime::String contentPath = Runtime::VFS::GetInstance()->AbsolutePath(Runtime::RuntimeCore::GetAssetsPathVFS());
		ContentBrowser::Create(contentPath);

		// 初始化资源导入管理器
		AssetImporterManager::GetInstance().Initialize(editorWindow);

		// 编辑器窗口
		{
			auto* editor = PanelManager::GetInstance();

			auto mainWindow = MakeRef<EditorMainWindow>();
			//auto editorSidebar = MakeRef<EditorSideBar>();

			editor->AddPanelWithID("EditorMenuBar", MakeRef<EditorMenuBar>(editorWindow.get()));
			//editor->AddPanelWithID("EditorSideBar", editorSidebar);
			editor->AddPanelWithID("EditorMainWindow", mainWindow);
			editor->AddPanelWithID("EditorPreferences", MakeRef<PreferencesPanel>());
			editor->AddPanelWithID("ProjectSetting", MakeRef<ProjectSettingPanel>());
			editor->AddPanelWithID("BuildSettings", MakeRef<BuildSettingsPanel>());

			mainWindow->AddPanelWithID("ContentBrowserPanel", MakeRef<ContentBrowserPanel>());
			mainWindow->AddPanelWithID("ConsolePanel", MakeRef<ConsolePanel>());
			mainWindow->AddPanelWithID("SceneBrowserPanel", MakeRef<SceneBrowserPanel>());
			mainWindow->AddPanelWithID("DetailBrowserPanel", MakeRef<DetailBrowserPanel>());
			mainWindow->AddPanelWithID("EditorViewport", MakeRef<EditorViewport>(gameEngine->GetViewport()));
			mainWindow->AddPanelWithID("CodeStudioPanel", MakeRef<CodeStudioPanel>());
		}
	}
}
