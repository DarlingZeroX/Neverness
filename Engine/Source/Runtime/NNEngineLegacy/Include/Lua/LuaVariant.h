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
#include <sol/object.hpp>
#include <unordered_map>

namespace VisionGal
{
	class LuaVariant
	{
	public:
		enum class Type {
			Nil,
			Boolean,
			Integer,
			Number,
			String,
			Table
		};

		LuaVariant() = default;

		// 从sol::object创建Variant
		static LuaVariant FromLuaObject(sol::object obj, sol::this_state lua);

		// 转换为当前Lua状态的object
		sol::object ToLuaObject(sol::this_state lua) const;

	private:
		Type type = Type::Nil;
		union {
			bool boolValue;
			int intValue;
			double doubleValue;
		};
		std::string stringValue;
		std::unordered_map<std::string, LuaVariant> tableValue;
	};
}