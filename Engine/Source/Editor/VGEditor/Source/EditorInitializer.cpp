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

#include "EditorInitializer.h"
#include <HFileSystem/Interface/HFileSystem.h>
#include <HCorePlatform/Include/NativeFileDialog/portable-file-dialogs.h>
#include <VGPackage/Include/PakWriter.h>
#include <VGEngine/Include/Core/Core.h>
#include <VGEngine/Include/Core/VFS.h>

bool EditorInitializer::CheckProjectRootDir(const std::string& projectRootDir)
{
	if (Horizon::HFileSystem::ExistsDirectory(projectRootDir) == false)
	{
		auto& preferences = VisionGal::Editor::EditorCore::GetEditorPreferences();
		if (preferences.Editor.EditorLanguage == "ZH-CN")
		{
			pfd::message("错误", "无效的项目位置: " + projectRootDir + "\n请用VGLauncher启动器打开正确项目位置",
				pfd::choice::ok, pfd::icon::error);
		}
		else
		{
			pfd::message("Error", "Invalid project location: " + projectRootDir + "\nPlease use the VGLauncher to open the correct project location",
				pfd::choice::ok, pfd::icon::error);
		}
		return false;
	}

	return true;
}

void EditorInitializer::PakResource(const EditorVFSPath& path)
{
	using namespace VisionGal;

	PakFileWriter writer;

	Horizon::HFileSystem::CreateDirectoryWhenNoExist("Data");

	if (Horizon::HFileSystem::ExistsFile("Data/engine.pak") == false)
		writer.WriteDirectoryToPakFile(path.engine, "Data/engine.pak", "");

	if (Horizon::HFileSystem::ExistsFile("Data/editor.pak") == false)
		writer.WriteDirectoryToPakFile(path.editor, "Data/editor.pak", "");
}

void EditorInitializer::InitializeVFS(const EditorVFSPath& path)
{
	using namespace VisionGal;

	//PakResource(path);

	auto& vfs = VFS::GetInstance();

	// 添加编辑器资源虚拟文件系统
	VFS::MountPackageFileSystem(
		Editor::EditorCore::GetEditorResourcePathVFS(),
		"Data/editor.pak",
		path.editor
	);

	// 添加引擎资源虚拟文件系统
	VFS::MountPackageFileSystem(
		Core::GetEngineResourcePathVFS(),
		"Data/engine.pak",
		path.engine
	);

	auto assetsFS = std::make_unique<vfspp::NativeFileSystem>(path.assets);
	auto projectSettingsFS = std::make_unique<vfspp::NativeFileSystem>(path.projectSettings);
	auto projectIntermediateFS = std::make_unique<vfspp::NativeFileSystem>(path.projectIntermediate);

	assetsFS->Initialize();
	projectSettingsFS->Initialize();
	projectIntermediateFS->Initialize();

	vfs->AddFileSystem(Core::GetAssetsPathVFS(), std::move(assetsFS));
	vfs->AddFileSystem(Core::GetProjectSettingsPathVFS(), std::move(projectSettingsFS));
	vfs->AddFileSystem(Core::GetProjectIntermediatePathVFS(), std::move(projectIntermediateFS));

	auto editorPath = VFS::GetInstance()->AbsolutePath(Editor::EditorCore::GetEditorResourcePathVFS());
	H_LOG_INFO("Editor resource path: %s", editorPath.c_str());

	auto enginePath = VFS::GetInstance()->AbsolutePath(Core::GetEngineResourcePathVFS());
	H_LOG_INFO("Engine resource path: %s", enginePath.c_str());

	auto assetsPath = VFS::GetInstance()->AbsolutePath(Core::GetAssetsPathVFS());
	H_LOG_INFO("Assets resource: %s", assetsPath.c_str());

	auto projectSettingsPath = VFS::GetInstance()->AbsolutePath(Core::GetProjectSettingsPathVFS());
	H_LOG_INFO("Project settings resource path: %s", projectSettingsPath.c_str());

	auto projectIntermediatePath = VFS::GetInstance()->AbsolutePath(Core::GetProjectIntermediatePathVFS());
	H_LOG_INFO("Project intermediate path: %s", projectIntermediatePath.c_str());
}
