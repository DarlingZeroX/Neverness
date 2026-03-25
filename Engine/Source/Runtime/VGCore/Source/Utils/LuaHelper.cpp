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

#include "Utils/LuaHelper.h"
#include <regex>

namespace VisionGal
{
	int LuaHelper::ExtractErrorLineNumber(const std::string& error_msg)
	{		// 多种可能的错误格式模式
		std::vector<std::regex> patterns = {
			std::regex(R"(\[string ".*"\]:(\d+):)"),          // [string "chunk"]:行号:
			std::regex(R"(\[string "\]:(\d+):)"),             // [string ]:行号:
			std::regex(R"(.lua:(\d+):)"),                     // filename.lua:行号:
			std::regex(R"(line (\d+):)")                      // line 行号:
		};

		std::smatch matches;

		for (const auto& pattern : patterns) {
			if (std::regex_search(error_msg, matches, pattern) && matches.size() > 1) {
				try {
					return std::stoi(matches[1].str());
				}
				catch (const std::exception& e) {
					//std::cout << "转换行号失败: " << e.what() << std::endl;
					break;
				}
			}
		}

		// 如果没有找到行号，输出完整错误信息用于调试
		H_LOG_INFO("未找到行号");
		return 0;
	}
}
