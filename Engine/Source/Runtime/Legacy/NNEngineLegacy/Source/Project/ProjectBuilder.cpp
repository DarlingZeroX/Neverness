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

#include "Project/ProjectBuilder.h"
#include "NNRuntimeAssetLegacy/Interface/Package.h"
#include "NNRuntimeCore/Include/Core/RuntimeCore.h"
#include "NNPlatformCore/Interface/FileSystem/HFileSystem.h"
#include "Project/ProjectSettings.h"
#include <NNRuntimePak/Include/PakWriter.h>

#include "NNRuntimeVFS/Include/VFSService.h"

namespace NN::Runtime
{
	bool ProjectBuilder::CheckBuildSettingIsValid(const BuildContext& context)
	{
		auto applicationMainScene = ProjectSettings::GetProjectSettings().Application.RunningMainScene;

		// 检查应用程序主场景是否有效
		VGAssetMetaData data;
		if (VGPackage::GetMeatData(applicationMainScene, data))
		{
			if (data.AssetType != "Scene")
			{
				H_LOG_ERROR("Application main scene is invalid!", applicationMainScene.c_str());
				return false;
			}

			return true;
		}

		// 检查生成目录是否为空
		if (context.OutputDirectory.empty())
		{
			H_LOG_ERROR("Build output directory cannot be empty!");
			return false;
		}

		// 生成目录是否有效
		if (NN::Core::HFileSystem::ExistsDirectory(context.OutputDirectory) == false)
		{
			H_LOG_ERROR("Build output directory is invalid!", context.OutputDirectory.c_str());
			return false;
		}

		// 生成目录必须为空目录
		if (NN::Core::HFileSystem::DirectoryEmpty(context.OutputDirectory) == false)
		{
			H_LOG_ERROR("Build output directory must be empty directory!", context.OutputDirectory.c_str());
			return false;
		}
		return true;
	}

	bool ProjectBuilder::WriteDirectoryToPakFile(const std::filesystem::path& InDirectoryPath,
		const std::filesystem::path& outPakPath)
	{
		PakFileWriter writer;
		writer.WriteDirectoryToPakFile(InDirectoryPath, outPakPath, "");

		return true;
	}

	bool ProjectBuilder::BuildProject(BuildContext& context)
	{
		if (CheckBuildSettingIsValid(context) == false)
			return false;

		// 保存项目设置
		ProjectSettings::SaveProjectSettings();

		std::filesystem::path outputPath(context.OutputDirectory);
		NN::Core::HFileSystem::CreateDirectoryWhenNoExist(outputPath);

		std::filesystem::path dataPath = outputPath / "Data";
		NN::Core::HFileSystem::CreateDirectoryWhenNoExist(dataPath);

		context.isError = false;
		context.process = 0.1f;

		// 打包引擎资源
		auto enginePath = VFS::VFSService::GetInstance()->AbsolutePath(RuntimeCore::GetEngineResourcePathVFS());
		H_LOG_INFO("Package engine resource: %s", enginePath.c_str());
		if (NN::Core::HFileSystem::ExistsFile("Data/engine.pak"))		// 应用程序模式
		{
			NN::Core::HFileSystem::CopyFile("Data/engine.pak", dataPath / "engine.pak");
		}
		else
		{
			if (WriteDirectoryToPakFile(enginePath, dataPath / "engine.pak") == false)
			{
				H_LOG_ERROR("Failed to write project data to pak file!");
				context.isError = true;
				return false;
			}
		}
		H_LOG_INFO("Successfully packaged engine resource!", enginePath.c_str());
		context.process = 0.2f;

		// 打包游戏资产
		auto assetsPath = VFS::VFSService::GetInstance()->AbsolutePath(RuntimeCore::GetAssetsPathVFS());
		H_LOG_INFO("Package assets: %s", assetsPath.c_str());
		if (NN::Core::HFileSystem::ExistsDirectory(assetsPath) == false)
		{
			context.isError = true;
			H_LOG_ERROR("Assets directory is invalid!", assetsPath.c_str());
			return false;
		}

		if (WriteDirectoryToPakFile(assetsPath, dataPath / "assets.pak") == false)
		{
			context.isError = true;
			H_LOG_ERROR("Failed to write project data to pak file!");
			return false;
		}

		H_LOG_INFO("Successfully packaged assets!", assetsPath.c_str());
		context.process = 0.7f;

		// 打包项目设置
		auto projectSettings = VFS::VFSService::GetInstance()->AbsolutePath(RuntimeCore::GetProjectSettingsPathVFS());
		H_LOG_INFO("Package project settings: %s", projectSettings.c_str());
		if (NN::Core::HFileSystem::ExistsDirectory(projectSettings) == false)
		{
			context.isError = true;
			H_LOG_ERROR("Project settings directory is invalid!", projectSettings.c_str());
			return false;
		}

		if (WriteDirectoryToPakFile(projectSettings, dataPath / "projectSetting.pak") == false)
		{
			context.isError = true;
			H_LOG_ERROR("Failed to write project data to pak file!");
			return false;
		}
		H_LOG_INFO("Successfully packaged project settings!", projectSettings.c_str());
		context.process = 0.8f; 
			
		// 跨平台复制当前目录库和VGDesktopApplication桌面应用程序

		// 遍历当前目录下的所有文件
		std::filesystem::path cwd = RuntimeCore::GetExecutableDirectory();
		H_LOG_INFO("Build Executable Directory: ",cwd.string().c_str());
		for (const auto& entry : std::filesystem::directory_iterator(cwd))
		{
			//std::cout << "Processing file: " << entry.path() << std::endl;
			if (entry.is_regular_file())
			{
				auto filePath = entry.path();
				if (filePath.extension() == ".dll" || filePath.extension() == ".so" || filePath.extension() == ".dylib")
				{
					NN::Core::HFileSystem::CopyFile(filePath, outputPath / filePath.filename());
				}

                // 跨平台复制VGDesktopApplication应用程序
                #if defined(_WIN32)
                if (filePath.filename() == "VGDesktopApplication.exe")
                #elif defined(__APPLE__)
                if (filePath.filename() == "VGDesktopApplication.app")
                #elif defined(__linux__)
                if (filePath.filename() == "VGDesktopApplication")
                #else
                // 其他平台可根据需要添加
                if (filePath.filename() == "VGDesktopApplication")
                #endif
                {
					NN::Core::HFileSystem::CopyFile(filePath, outputPath / filePath.filename());
                }
			}
		}
		context.process = 1.0f;
		return true;
	}


}
