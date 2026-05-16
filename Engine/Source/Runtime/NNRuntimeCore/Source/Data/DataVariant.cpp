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

#include "Data/DataVariant.h"
#include <sol/sol.hpp>

namespace NN::Runtime
{
	VGDataVariant::VGDataVariant(int value)
	{
		SetValue(value);
	}

	VGDataVariant::VGDataVariant(bool value)
	{
		SetValue(value);
	}

	VGDataVariant::VGDataVariant(double value)
	{
		SetValue(value);
	}

	VGDataVariant::VGDataVariant(const std::string& value)
	{
		SetValue(value);
	}

	VGDataVariant VGDataVariant::FromLuaObject(sol::object obj, sol::this_state lua)
	{
		VGDataVariant variant;

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

	sol::object VGDataVariant::ToLuaObject(sol::this_state lua) const
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

	void VGDataVariant::SetValue(int value)
	{
		type = Type::Integer;
		intValue = value;
	}

	void VGDataVariant::SetValue(bool value)
	{
		type = Type::Boolean;
		boolValue = value;
	}

	void VGDataVariant::SetValue(double value)
	{
		type = Type::Number;
		doubleValue = value;
	}

	void VGDataVariant::SetValue(const std::string& value)
	{
		type = Type::String;
		stringValue = value;
	}

	int VGDataVariant::GetValueInt() const
	{
		return GetValue<int>();
	}

	bool VGDataVariant::GetValueBool() const
	{
		return GetValue<bool>();
	}

	double VGDataVariant::GetValueNumber() const
	{
		return GetValue<double>();
	}

	std::string VGDataVariant::GetValueString() const
	{
		return GetValue<std::string>();
	}

	void VGDataVariant::Serialize(const std::string& name, nlohmann::json& json)
	{
		nlohmann::json varJson;
		varJson["Type"] = type;

		switch (type) {
		case Type::Boolean:
			varJson["Value"] = boolValue;
			break;
		case Type::Integer:
			varJson["Value"] = intValue;
			break;
		case Type::Number:
			varJson["Value"] = doubleValue;
			break;
		case Type::String:
			varJson["Value"] = stringValue;
			break;
		}

		json[name] = varJson;
	}

	void VGDataVariant::Deserialize(nlohmann::json& json)
	{
		type = json["Type"];
		switch (type) {
		case Type::Boolean:
			boolValue = json["Value"];
			break;
		case Type::Integer:
			intValue = json["Value"];
			break;
		case Type::Number:
			doubleValue = json["Value"];
			break;
		case Type::String:
			stringValue = json["Value"];
			break;
		}
	}
}
