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

#include "Editor/EditorInitializer.h"
#include "Editor/EditorCore.h"

#include <NNFileSystem/Interface/HFileSystem.h>
#include <NNPlatformCore/Include/NativeFileDialog/portable-file-dialogs.h>
#include <NNRuntimePak/Include/PakWriter.h>
#include <NNRuntimeCore/Include/Core/RuntimeCore.h>
#include "NNRuntimeVFS/Include/VFSService.h"
#include "NNRuntimePak/Include/VFSMount.h"
#include "NNRuntimeVFS/Include/VFS/NativeFileSystem.h"

namespace NN::Runtime::Application
{

	bool EditorInitializer::CheckProjectRootDir(const std::string& projectRootDir)
	{
		if (NN::Core::HFileSystem::ExistsDirectory(projectRootDir) == false)
		{
			//auto& preferences = NN::Editor::EditorCore::GetEditorPreferences();
			//if (preferences.Editor.EditorLanguage == "ZH-CN")
			//{
			//	pfd::message("错误", "无效的项目位置: " + projectRootDir + "\n请用VGLauncher启动器打开正确项目位置",
			//		pfd::choice::ok, pfd::icon::error);
			//}
			//else
			{
				pfd::message("Error", "Invalid project location: " + projectRootDir + "\nPlease use the NNLauncher to open the correct project location",
					pfd::choice::ok, pfd::icon::error);
			}
			return false;
		}

		return true;
	}

	void EditorInitializer::PakResource(const EditorVFSPath& path)
	{
		//using namespace VisionGal;

		NN::Runtime::PakFileWriter writer;

		NN::Core::HFileSystem::CreateDirectoryWhenNoExist("Data");

		if (NN::Core::HFileSystem::ExistsFile("Data/engine.pak") == false)
			writer.WriteDirectoryToPakFile(path.engine, "Data/engine.pak", "");

		if (NN::Core::HFileSystem::ExistsFile("Data/editor.pak") == false)
			writer.WriteDirectoryToPakFile(path.editor, "Data/editor.pak", "");
	}

	void EditorInitializer::InitializeVFS(const EditorVFSPath& path)
	{
		//using namespace VisionGal;

		//PakResource(path);

		auto& vfs = NN::Runtime::VFS::VFSService::GetInstance();

		// 添加编辑器资源虚拟文件系统
		NN::Runtime::VFS::MountPackageFileSystem(
			NN::Editor::EditorCore::GetEditorResourcePathVFS(),
			"Data/editor.pak",
			path.editor
		);

		// 添加引擎资源虚拟文件系统
		NN::Runtime::VFS::MountPackageFileSystem(
			NN::Runtime::RuntimeCore::GetEngineResourcePathVFS(),
			"Data/engine.pak",
			path.engine
		);

		auto assetsFS = std::make_unique<NN::Runtime::VFS::NativeFileSystem>(path.assets);
		auto projectSettingsFS = std::make_unique<NN::Runtime::VFS::NativeFileSystem>(path.projectSettings);
		auto projectIntermediateFS = std::make_unique<NN::Runtime::VFS::NativeFileSystem>(path.projectIntermediate);

		assetsFS->Initialize();
		projectSettingsFS->Initialize();
		projectIntermediateFS->Initialize();

		vfs->AddFileSystem(NN::Runtime::RuntimeCore::GetAssetsPathVFS(), std::move(assetsFS));
		vfs->AddFileSystem(NN::Runtime::RuntimeCore::GetProjectSettingsPathVFS(), std::move(projectSettingsFS));
		vfs->AddFileSystem(NN::Runtime::RuntimeCore::GetProjectIntermediatePathVFS(), std::move(projectIntermediateFS));

		auto editorPath = NN::Runtime::VFS::VFSService::GetInstance()->AbsolutePath(NN::Editor::EditorCore::GetEditorResourcePathVFS());
		H_LOG_INFO("Editor resource path: %s", editorPath.c_str());

		auto enginePath = NN::Runtime::VFS::VFSService::GetInstance()->AbsolutePath(NN::Runtime::RuntimeCore::GetEngineResourcePathVFS());
		H_LOG_INFO("Engine resource path: %s", enginePath.c_str());

		auto assetsPath = NN::Runtime::VFS::VFSService::GetInstance()->AbsolutePath(NN::Runtime::RuntimeCore::GetAssetsPathVFS());
		H_LOG_INFO("Assets resource: %s", assetsPath.c_str());

		auto projectSettingsPath = NN::Runtime::VFS::VFSService::GetInstance()->AbsolutePath(NN::Runtime::RuntimeCore::GetProjectSettingsPathVFS());
		H_LOG_INFO("Project settings resource path: %s", projectSettingsPath.c_str());

		auto projectIntermediatePath = NN::Runtime::VFS::VFSService::GetInstance()->AbsolutePath(NN::Runtime::RuntimeCore::GetProjectIntermediatePathVFS());
		H_LOG_INFO("Project intermediate path: %s", projectIntermediatePath.c_str());
	}
}
