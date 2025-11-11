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

#include "Lua/LuaVariant.h"

namespace VisionGal
{
	LuaVariant LuaVariant::FromLuaObject(sol::object obj, sol::this_state lua)
	{
		LuaVariant variant;

		if (obj.is<sol::nil_t>()) {
			variant.type = Type::Nil;
		}
		else if (obj.is<bool>()) {
			variant.type = Type::Boolean;
			variant.boolValue = obj.as<bool>();
		}
		else if (obj.is<int>()) {
			variant.type = Type::Integer;
			variant.intValue = obj.as<int>();
		}
		else if (obj.is<double>()) {
			variant.type = Type::Number;
			variant.doubleValue = obj.as<double>();
		}
		else if (obj.is<std::string>()) {
			variant.type = Type::String;
			variant.stringValue = obj.as<std::string>();
		}
		else if (obj.is<sol::table>()) {
			// 递归处理表（简化版，实际需要更复杂的处理）
			variant.type = Type::Table;
			sol::table table = obj.as<sol::table>();
			for (auto& pair : table) {
				variant.tableValue[pair.first.as<std::string>()] = 
					FromLuaObject(pair.second, lua);
			}
		}

		return variant;
	}

	sol::object LuaVariant::ToLuaObject(sol::this_state lua) const
	{
		sol::state_view lua_view(lua);

		switch (type) {
		case Type::Nil:
			return sol::make_object(lua, sol::nil);
		case Type::Boolean:
			return sol::make_object(lua, boolValue);
		case Type::Integer:
			return sol::make_object(lua, intValue);
		case Type::Number:
			return sol::make_object(lua, doubleValue);
		case Type::String:
			return sol::make_object(lua, stringValue);
		case Type::Table:
		{
			sol::table table = lua_view.create_table();
			for (const auto& pair : tableValue) {
				table[pair.first] = pair.second.ToLuaObject(lua);
			}
			return table;
		}
		default:
			return sol::make_object(lua, sol::nil);
		}
	}
}
