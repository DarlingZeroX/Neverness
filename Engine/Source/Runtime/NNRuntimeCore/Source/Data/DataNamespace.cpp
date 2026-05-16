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

#include "Data/DataNamespace.h"
#include <sol/sol.hpp>

namespace NN::Runtime
{
	sol::object VGDataNamespace::GetVariableLua(const String& name, sol::this_state lua)
	{
		if (const auto it = m_Data.find(name); it != m_Data.end())
		{
			// 将存储的Variant转换为当前Lua状态的object
			return it->second.ToLuaObject(lua);
		}
		return sol::make_object(lua, sol::nil);
	}

	void VGDataNamespace::SetVariableLua(const String& name, sol::object srcObj, sol::this_state luaState)
	{
		// nil
		if (!srcObj.valid() || srcObj.is<sol::nil_t>())
		{
			m_Data[name] = VGDataVariant();
		}

		// 将sol::object转换为与状态无关的Variant存储
		m_Data[name] = VGDataVariant::FromLuaObject(srcObj, luaState);
	}

	bool VGDataNamespace::ExistVariable(const std::string& name) const
	{
		return m_Data.find(name) != m_Data.end();
	}

	bool VGDataNamespace::RemoveVariable(const std::string& name)
	{
		return m_Data.erase(name) > 0;
	}

	void VGDataNamespace::Clear()
	{
		m_Data.clear();
	}

	void VGDataNamespace::Serialize(nlohmann::json& json)
	{
		for (auto& [name, var] : m_Data)
		{
			var.Serialize(name, json);
		}
	}

	void VGDataNamespace::Deserialize(nlohmann::json& json)
	{
		for (const auto& [name, varJson] : json.items())
		{
			VGDataVariant varData;
			varData.Deserialize(varJson);
			m_Data[name] = varData;
		}
	}
}
