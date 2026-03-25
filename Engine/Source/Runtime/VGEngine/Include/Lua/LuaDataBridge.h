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
#include "../EngineConfig.h"
#include <sol/object.hpp>
#include <unordered_map>
#include "LuaVariant.h"
#include <VGCore/Include/Core/Core.h>

namespace VisionGal
{
	class VG_ENGINE_API LuaDataBridge
	{
	public:
		sol::object GetVariable(const String& name, sol::this_state lua);
		void SetVariable(const String& name, sol::object obj, sol::this_state lua);
	private:
		// 使用自定义的Variant类型存储，而不是直接存储sol::object
		std::unordered_map<String, LuaVariant> m_Data;
	};

	class VG_ENGINE_API LuaDataBridgeManager
	{
	public:
		LuaDataBridgeManager();
		~LuaDataBridgeManager() = default;

		static LuaDataBridgeManager* GetInstance();

		LuaDataBridge* GetDataBridge(const std::string& name);
	private:
		std::unordered_map<String, Ref<LuaDataBridge>> m_DateBridges;
	};

	struct LuaDataBridgeLuaInterface
	{
		static void Initialise(sol::state& L);
	};
}