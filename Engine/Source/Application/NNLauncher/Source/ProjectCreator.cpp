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

#include "ProjectCreator.h"
#include <filesystem>

#include "VGLauncherData.h"
#include "NNKernel/Include/Math/HVector.h"
#include "NNFileSystem/Interface/HFileSystem.h"

namespace VisionGal::Editor
{
	// 创建新项目的函数
	bool ProjectCreator::CreateProject(const ProjectCreateConfig& config)
	{
		// 检查项目名称和路径是否为空
		if (config.ProjectName.empty() || config.ProjectPath.empty())
			return false;

		// 检查项目路径是否存在
		if (Horizon::HFileSystem::ExistsDirectory(config.ProjectPath) == false)
			return false;

		// 检查同名项目是否已存在
		std::filesystem::path path = config.ProjectPath;
		if (std::filesystem::exists(path / config.ProjectName))
			return false;

		// 获取启动器数据的单例实例
		auto& data = VGLauncherData::GetLauncherData();

		// 构建项目的完整路径
		std::filesystem::path projectPath(config.ProjectPath);
		projectPath /= config.ProjectName;

		// 创建项目主目录
		std::filesystem::create_directories(projectPath);

		// 创建 Assets 资源目录
		std::filesystem::path assetPath = projectPath / "Assets";
		std::filesystem::create_directories(assetPath);

		// 创建 ProjectSettings 配置目录
		std::filesystem::path projectSettings = projectPath / "ProjectSettings";
		std::filesystem::create_directories(projectSettings);

		// 创建 Editor 目录
		std::filesystem::path editorPath = projectPath / "Editor";
		std::filesystem::create_directories(editorPath);

		// 创建 Intermediate 目录
		std::filesystem::path intermediatePath = projectPath / "Intermediate";
		std::filesystem::create_directories(intermediatePath);

		// 创建项目项并添加到启动器数据中
		VGProjectItem project;
		project.Name = config.ProjectName;
		project.Path = projectPath.string();
		data.AddProject(project);

		// 返回创建成功
		return true;
	}
}
