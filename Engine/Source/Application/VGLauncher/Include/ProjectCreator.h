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
#include <string>
#include <vector>

namespace VisionGal::Editor
{
	struct ProjectCreateConfig
	{
		/// @brief 项目名称
		std::string ProjectName;
		/// @brief 项目路径
		std::string ProjectPath;
	};

	struct ProjectCreator
	{
		/// @brief 创建一个新项目。
		/// @return 如果项目创建成功，则返回 true；否则返回 false。
		static bool CreateProject(const ProjectCreateConfig& config);
	};
}
