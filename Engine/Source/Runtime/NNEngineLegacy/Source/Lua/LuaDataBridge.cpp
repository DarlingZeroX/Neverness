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

#include "Lua/LuaDataBridge.h"
#include "NNRuntimeCore/Include/Core/EventBus.h"
#include "Render/TransitionManager.h"
#include <sol/sol.hpp>

namespace NN::Runtime
{
	sol::object LuaDataBridge::GetVariable(const String& name, sol::this_state lua)
	{
		if (const auto it = m_Data.find(name); it != m_Data.end())
		{
			// 将存储的Variant转换为当前Lua状态的object
			return it->second.ToLuaObject(lua);
		}
		return sol::make_object(lua, sol::nil);
	}

	void LuaDataBridge::SetVariable(const String& name, sol::object srcObj, sol::this_state luaState)
	{
		// nil
		if (!srcObj.valid() || srcObj.is<sol::nil_t>())
		{
			m_Data[name] = LuaVariant();
		}

		// 将sol::object转换为与状态无关的Variant存储
		m_Data[name] = LuaVariant::FromLuaObject(srcObj, luaState);
	}

	LuaDataBridgeManager::LuaDataBridgeManager()
	{
		EngineEventBus::Get().OnEngineEvent.Subscribe([this](const EngineEvent& evt)
			{
				switch (evt.EventType)
				{
				case EngineEventType::EnterScenePlayMode:
					m_DateBridges.clear();
					break;
				case EngineEventType::ExitScenePlayMode:
					m_DateBridges.clear();
					break;
				}
			});
	}

	LuaDataBridgeManager* LuaDataBridgeManager::GetInstance()
	{
		static LuaDataBridgeManager instance;
		return &instance;
	}

	LuaDataBridge* LuaDataBridgeManager::GetDataBridge(const std::string& name)
	{
		if (m_DateBridges.find(name) != m_DateBridges.end())
		{
			return m_DateBridges[name].get();
		}

		Ref<LuaDataBridge> bridge = std::make_shared<LuaDataBridge>();
		m_DateBridges[name] = bridge;
		return bridge.get();
	}

	void LuaDataBridgeLuaInterface::Initialise(sol::state& lua)
	{
		lua.new_usertype<LuaDataBridge>("LuaDataBridge",
			"设置变量", &LuaDataBridge::SetVariable,
			"获取变量", &LuaDataBridge::GetVariable,
			sol::meta_function::index, [](LuaDataBridge& self, const std::string& key, sol::this_state state) {
				return self.GetVariable(key, state);
			},
			sol::meta_function::new_index, [](LuaDataBridge& self, const std::string& key, const sol::object& value, sol::this_state state) {
				return self.SetVariable(key, value, state);
			}
		);

		lua.new_usertype<LuaDataBridgeManager>("LuaDataBridgeManager",
			"获取数据桥", &LuaDataBridgeManager::GetDataBridge
		);

		lua["LuaDataBridgeManager"] = sol::make_object(lua, LuaDataBridgeManager::GetInstance());
		lua["数据桥管理器"] = sol::make_object(lua, LuaDataBridgeManager::GetInstance());
	}
}
