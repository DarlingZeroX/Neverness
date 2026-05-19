// Copyright (c) 2025 梦旅缘心
// This file is part of VisionGal and is licensed under the MIT License.
// For the latest information, see https://github.com/DarlingZeroX/VisionGal
// See the LICENSE file in the project root for details.

#pragma once
#include "ProjectCreator.h"
#include <filesystem>
#include <gtest/gtest.h>

using namespace VisionGal::Editor;

TEST(ProjectCreatorTest, CreateProject) {

	// 测试环境准备
	std::filesystem::path testPath = "TestDirectory/VGLauncherTest/CreateProject";
	std::filesystem::remove_all(testPath);
	std::filesystem::create_directories(testPath);

	// 空配置，应该失败
	{
		ProjectCreateConfig config;
		ASSERT_FALSE(ProjectCreator::CreateProject(config));
	}

	// 空项目路径，应该失败
	{
		ProjectCreateConfig config;
		config.ProjectName = "TestProject";
		ASSERT_FALSE(ProjectCreator::CreateProject(config));
	}

	// 不存在项目路径，应该失败
	{
		ProjectCreateConfig config;
		config.ProjectName = "TestProject";
		config.ProjectPath = "123456789123456789123456789";
		ASSERT_FALSE(ProjectCreator::CreateProject(config));
	}

	// 空项目名称，应该失败
	{
		ProjectCreateConfig config;
		config.ProjectName = "";
		config.ProjectPath = testPath.string();
		ASSERT_FALSE(ProjectCreator::CreateProject(config));
	}

	// 正常配置，应该成功
	{
		ProjectCreateConfig config;
		config.ProjectName = "TestProject";
		config.ProjectPath = testPath.string();
		ASSERT_TRUE(ProjectCreator::CreateProject(config));
		std::filesystem::path projectPath = testPath / config.ProjectName;
		ASSERT_TRUE(std::filesystem::exists(projectPath));
		ASSERT_TRUE(std::filesystem::exists(projectPath / "Assets"));
		ASSERT_TRUE(std::filesystem::exists(projectPath / "ProjectSettings"));
		ASSERT_TRUE(std::filesystem::exists(projectPath / "Editor"));
		ASSERT_TRUE(std::filesystem::exists(projectPath / "Intermediate"));
	}

	// 同名项目已存在，应该失败
	{
		ProjectCreateConfig config;
		config.ProjectName = "TestProject";
		config.ProjectPath = testPath.string();
		ASSERT_FALSE(ProjectCreator::CreateProject(config));
	}

	// 中文正常配置，应该成功
	{
		ProjectCreateConfig config;
		config.ProjectName = "测试项目";
		config.ProjectPath = testPath.string();
		ASSERT_TRUE(ProjectCreator::CreateProject(config));
		std::filesystem::path projectPath = testPath / config.ProjectName;
		ASSERT_TRUE(std::filesystem::exists(projectPath));
		ASSERT_TRUE(std::filesystem::exists(projectPath / "Assets"));
		ASSERT_TRUE(std::filesystem::exists(projectPath / "ProjectSettings"));
		ASSERT_TRUE(std::filesystem::exists(projectPath / "Editor"));
		ASSERT_TRUE(std::filesystem::exists(projectPath / "Intermediate"));
	}

	// 清理测试环境
	std::filesystem::remove_all(testPath);
}
