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

#include "Galgame/Core/ArchiveDataContainer.h"
#include <sol/sol.hpp>

namespace VisionGal::GalGame
{
	VGDataNamespace* ArchiveDataContainer::GetChoicesNamespace()
	{
		return GetNamespace("__Choices__");
	}

	VGDataNamespace* ArchiveDataContainer::GetInputNamespace()
	{
		return GetNamespace("__Inputs__");
	}

	void ArchiveDataContainer::InitializeLuaBinding(sol::table& lua)
	{
		lua.new_usertype<VGDataNamespace>("VGDataNamespace",
			"设置变量", &VGDataNamespace::SetVariableLua,
			"获取变量", &VGDataNamespace::GetVariableLua,
			sol::meta_function::index, [](VGDataNamespace& self, const std::string& key, sol::this_state state) {
				return self.GetVariableLua(key, state);
			},
			sol::meta_function::new_index, [](VGDataNamespace& self, const std::string& key, const sol::object& value, sol::this_state state) {
				return self.SetVariableLua(key, value, state);
			}
		);

		lua.new_usertype<ArchiveDataContainer>("ArchiveDataContainer",
			"获取命名空间", &ArchiveDataContainer::GetNamespace,
			"剧情选择", sol::property(
				[](ArchiveDataContainer& self) -> VGDataNamespace* { return self.GetChoicesNamespace(); }
			),
			"文本输入", sol::property(
				[](ArchiveDataContainer& self) -> VGDataNamespace* { return self.GetInputNamespace(); }
			),
			sol::meta_function::index, [](ArchiveDataContainer& self, const std::string& key, sol::this_state state) {
				return self.GetNamespace(key);
			}
		);
	}
}

