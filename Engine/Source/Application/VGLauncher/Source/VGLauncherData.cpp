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

#include "VGLauncherData.h"
#include <HCore/Include/File/nlohmann/json.hpp>
#include <HFileSystem/Interface/HFileSystem.h>

namespace VisionGal::Editor
{
	struct VGLauncherDataImp
	{
		VGLauncherDataImp()
		{

		}

		~VGLauncherDataImp()
		{
			VGLauncherData::SaveLauncherData();
		}

		static VGLauncherDataImp* GetInstance()
		{
			static VGLauncherDataImp imp;
			return &imp;
		}

		void Initialize()
		{
		}

		/// @brief 全局启动器数据
		VGLauncherData m_VGLauncherData;
	};

	/// @brief 将 VGProjectItem 转换为 JSON
	void to_json(nlohmann::json& j, const VGProjectItem& item)
	{
		j = nlohmann::json{
			{"Name", item.Name},
			{"Path", item.Path}
		};
	}

	/// @brief 将 JSON 转换为 VGProjectItem
	void from_json(const nlohmann::json& j, VGProjectItem& item)
	{
		j.at("Name").get_to(item.Name);
		j.at("Path").get_to(item.Path);
	}

	// @brief 将 VGLauncherData 转换为 JSON
	void to_json(nlohmann::json& j, const VGLauncherData& data)
	{
		j = nlohmann::json{
			{"LastProjectCreateDirectory", data.m_LastProjectCreateDirectory},
			{"Projects", data.m_Projects}
		};
	}
	
	/// @brief 将 JSON 转换为 VGLauncherData
	void from_json(const nlohmann::json& j, VGLauncherData& data)
	{
		j.at("LastProjectCreateDirectory").get_to(data.m_LastProjectCreateDirectory);
		j.at("Projects").get_to(data.m_Projects);
	}

	VGLauncherData::VGLauncherData()
	{
		m_LauncherDataFilePath = "Data/VGLauncher/VGLauncherData.json";
	}

	VGLauncherData::VGLauncherData(const std::filesystem::path& dataPath)
	{
		m_LauncherDataFilePath = dataPath;
	}

	bool VGLauncherData::Load(VGLauncherData& data)
	{
		/// @brief 从文件加载启动器数据
		auto dataFilePath = data.m_LauncherDataFilePath;
		std::string text;
		if (Horizon::HFileSystem::ReadTextFromFile(data.m_LauncherDataFilePath, text))
		{
			// 尝试解析 JSON 数据
			try {
				nlohmann::json json = nlohmann::json::parse(text);
				data = json.get<VGLauncherData>();
				data.m_LauncherDataFilePath = dataFilePath;

				// 移除无效的项目
				data.RemoveInvalidProjects();
				return true;
			}
			catch (const nlohmann::json::parse_error& e) {

			}
			catch (...)
			{

			}
		}

		return false;
	}

	bool VGLauncherData::Save(VGLauncherData& data)
	{
		// 如果目录不存在则创建
		Horizon::HFileSystem::CreateDirectoryWhenNoExist("Data");
		Horizon::HFileSystem::CreateDirectoryWhenNoExist("Data/VGLauncher");

		// 将数据转换为 JSON 字符串
		nlohmann::json json;
		json = data;
		std::string jsonStr = json.dump(2);

		// 将 JSON 字符串写入文件
		return Horizon::HFileSystem::WriteTextToFile(data.m_LauncherDataFilePath, jsonStr);
	}

	void VGLauncherData::LoadLauncherData()
	{
		Load(VGLauncherDataImp::GetInstance()->m_VGLauncherData);
	}

	void VGLauncherData::SaveLauncherData()
	{
		Save(VGLauncherDataImp::GetInstance()->m_VGLauncherData);
	}

	VGLauncherData& VGLauncherData::GetLauncherData()
	{
		return VGLauncherDataImp::GetInstance()->m_VGLauncherData;
	}

	bool VGLauncherData::AddProject(const VGProjectItem& project)
	{
		// 检查项目是否已存在
		for (const auto& existingProject : m_Projects)
		{
			if (existingProject.Path == project.Path)
			{
				// 如果项目已存在，则不添加
				return false;
			}
		}

		if (Horizon::HFileSystem::ExistsDirectory(project.Path) == false)
			return false;

		m_Projects.push_back(project);

		return true;
	}

	bool VGLauncherData::RemoveProject(int index)
	{
		if (index < 0)
			return false;

		if (index >= m_Projects.size())
			return false;

		// 删除项目
		m_Projects.erase(m_Projects.begin() + index);
	}

	const std::vector<VGProjectItem>& VGLauncherData::GetProjects() const
	{
		return m_Projects;
	}

	const std::string& VGLauncherData::GetLastProjectCreateDirectory() const
	{
		return m_LastProjectCreateDirectory;
	}

	void VGLauncherData::SetLastProjectCreateDirectory(const std::string& directory)
	{
		m_LastProjectCreateDirectory = directory;
	}

	// 伪代码/实现计划（详尽）：
	// 1. 使用 std::filesystem 检查传入的 directory 是否存在且为目录。
	// 2. 如果不存在或不是目录，返回 false。
	// 3. 使用 std::filesystem::directory_iterator 遍历目录下的直接子项。
	// 4. 对于每个子项，判断是否为目录（is_directory）。
	// 5. 对目录子项：
	//    a. 使用 path.filename().string() 作为项目名称（与之前 HFileSystem 的行为一致）。
	//    b. 使用 path.string() 作为项目路径。
	//    c. 构造 VGProjectItem 并调用 AddProject 添加（AddProject 内部会去重并验证路径）。
	// 6. 任何文件系统错误（通过 std::error_code 捕获）将导致函数返回 false。
	// 7. 遍历完成后返回 true（即使未添加任何项目也返回 true，行为与原注释代码一致）。
	bool VGLauncherData::LoadAllProjectsInDirectory(const std::string& directory)
	{
		std::error_code ec;

		// 检查目录是否存在并且是目录
		if (!std::filesystem::exists(directory, ec) || ec)
			return false;

		if (!std::filesystem::is_directory(directory, ec) || ec)
			return false;

		// 遍历目录下的直接子目录
		for (auto it = std::filesystem::directory_iterator(directory, ec); it != std::filesystem::directory_iterator(); it.increment(ec))
		{
			if (ec)
			{
				// 遍历过程中发生错误
				return false;
			}

			const auto& entry = *it;
			// 仅处理目录
			if (!entry.is_directory(ec) || ec)
				continue;

			const auto& path = entry.path();
			// 使用目录名称作为项目名称（不包含扩展）
			std::string projectName = path.filename().string();
			std::string projectPath = path.string();

			VGProjectItem projectItem;
			projectItem.Name = projectName;
			projectItem.Path = projectPath;

			// AddProject 会检查重复以及目录有效性
			AddProject(projectItem);
		}

		return true;
	}

	void VGLauncherData::RemoveInvalidProjects()
	{
		// 移除路径不存在的项目
		m_Projects.erase(std::remove_if(m_Projects.begin(), m_Projects.end(),
			[](const VGProjectItem& project)
			{
				return !Horizon::HFileSystem::ExistsDirectory(project.Path);
			}), m_Projects.end());
	}
}
