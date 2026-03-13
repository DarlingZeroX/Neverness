// Copyright (c) 2025 梦旅缘心
// This file is part of VisionGal and is licensed under the MIT License.
// For the latest information, see https://github.com/DarlingZeroX/VisionGal
// See the LICENSE file in the project root for details.

#pragma once
#include "ProjectCreator.h"
#include "VGLauncherData.h"
#include <filesystem>
#include <gtest/gtest.h>

using namespace VisionGal::Editor;

TEST(VGLauncherDataTest, TestData) {

	// 测试环境准备
	std::filesystem::path testPath = "TestDirectory/VGLauncherTest/VGLauncherData";
	std::filesystem::remove_all(testPath);
	std::filesystem::create_directories(testPath);

	int testProjectNumber = 50;

	// 创建测试项目
	for (int i = 0; i < testProjectNumber; i++)
	{
		ProjectCreateConfig config;
		config.ProjectName = "TestProject" + std::to_string(i);
		config.ProjectPath = testPath.string();
		ASSERT_TRUE(ProjectCreator::CreateProject(config));
	}

	{
		std::filesystem::path path = testPath / "VGLauncherData.json";
		VGLauncherData data(path);
		VGLauncherData data1(path);
		// 初始数据应该为空
		EXPECT_EQ(data.GetProjects().size(), 0);

		// 加载不存在的数据文件，应该失败
		ASSERT_FALSE(VGLauncherData::Load(data));
		// 保存数据，应该成功
		ASSERT_TRUE(VGLauncherData::Save(data));
		// 再次加载数据，应该成功
		ASSERT_TRUE(VGLauncherData::Load(data));

		// 测试添加不存在项目
		{
			VGProjectItem item;
			item.Name = "NotExistProject111";
			item.Path = (testPath / item.Name).string();
			ASSERT_FALSE(data.AddProject(item));
		}

		// 项目此时应该还是空的
		EXPECT_EQ(data.GetProjects().size(), 0);

		// 测试添加项目
		for (int i = 0; i < testProjectNumber; i++)
		{
			VGProjectItem item;
			item.Name = "TestProject" + std::to_string(i);
			item.Path = (testPath / item.Name).string();
			ASSERT_TRUE(data.AddProject(item));
		}

		// 项目此时应该有 50 个
		EXPECT_EQ(data.GetProjects().size(), testProjectNumber);

		// 测试重复添加项目
		for (int i = 0; i < testProjectNumber; i++)
		{
			VGProjectItem item;
			item.Name = "TestProject" + std::to_string(i);
			item.Path = (testPath / item.Name).string();
			ASSERT_FALSE(data.AddProject(item));
		}

		// 项目此时应该有 50 个
		EXPECT_EQ(data.GetProjects().size(), testProjectNumber);

		// 删除前 25 个项目
		for (int i = 0; i < testProjectNumber / 2; i++)
		{
			ASSERT_TRUE(data.RemoveProject(0));
		}

		// 项目此时应该有 25 个
		EXPECT_EQ(data.GetProjects().size(), testProjectNumber / 2);

		// 删除后 25 个项目
		for (int i = 0; i < testProjectNumber / 2; i++)
		{
			ASSERT_TRUE(data.RemoveProject(0));
		}

		// 项目此时应该没有项目了
		EXPECT_EQ(data.GetProjects().size(), 0);

		// 再次添加项目
		for (int i = 0; i < testProjectNumber; i++)
		{
			VGProjectItem item;
			item.Name = "TestProject" + std::to_string(i);
			item.Path = (testPath / item.Name).string();
			ASSERT_TRUE(data.AddProject(item));
		}

		// 项目此时应该有 50 个
		EXPECT_EQ(data.GetProjects().size(), testProjectNumber);

		// 设置最后创建项目路径
		data.SetLastProjectCreateDirectory(testPath.string());

		// 保存数据，应该成功
		ASSERT_TRUE(VGLauncherData::Save(data));

		// 用另一个数据实例读取数据
		{
			// 再次读取数据，应该成功
			ASSERT_TRUE(VGLauncherData::Load(data1));

			// 检查最后创建项目路径是否一致
			EXPECT_EQ(data.GetLastProjectCreateDirectory(), data1.GetLastProjectCreateDirectory());

			// 项目此时应该有 50 个
			EXPECT_EQ(data1.GetProjects().size(), testProjectNumber);

			// 检查数据是否一致
			for (int i = 0; i < testProjectNumber; i++)
			{
				EXPECT_EQ(data.GetProjects()[i].Name, data1.GetProjects()[i].Name);
				EXPECT_EQ(data.GetProjects()[i].Path, data1.GetProjects()[i].Path);
			}

			// 测试重复添加项目
			for (int i = 0; i < testProjectNumber; i++)
			{
				VGProjectItem item;
				item.Name = "TestProject" + std::to_string(i);
				item.Path = (testPath / item.Name).string();
				ASSERT_FALSE(data1.AddProject(item));
			}

			// 项目此时应该有 50 个
			EXPECT_EQ(data1.GetProjects().size(), testProjectNumber);
		}
	}

	// 清理测试环境
	std::filesystem::remove_all(testPath);
}
