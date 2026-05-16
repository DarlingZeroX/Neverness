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

#include "SolPlugin.h"
#include <RmlUi/Core/Factory.h>
#include <RmlUi/Core/FileInterface.h>
#include <RmlUi/Core/Log.h>
#include "Types.h"

namespace RmlSol {

	static NN::Ref<sol::state> g_L = nullptr;

	SolPlugin::SolPlugin(NN::Ref<sol::state> lua_state)
	{
		RMLUI_ASSERT(g_L == nullptr);
		g_L = lua_state;
	}

	int SolPlugin::GetEventClasses()
	{
		return Rml::Plugin::EVT_BASIC;
	}

	void SolPlugin::OnInitialise()
	{
		//if (g_L == nullptr)
		//{
		//	Rml::Log::Message(Rml::Log::LT_INFO, "Loading Lua plugin using a new Lua state.");
		//	g_L = new sol::state();
		//	g_L->open_libraries(sol::lib::base,
		//		sol::lib::math,
		//		sol::lib::string,
		//		sol::lib::table); // 默认已加载这些库
		//	owns_lua_state = true;
		//}
		//else
		//{
		//	//Rml::Log::Message(Rml::Log::LT_INFO, "Loading Lua plugin using the provided Lua state.");
		//	//owns_lua_state = false;
		//	Rml::Log::Message(Rml::Log::LT_INFO, "Loading Lua plugin using a exist Lua state.");
		//	owns_lua_state = false;
		//}
		//RegisterTypes();
		RebindLuaState(g_L);

		lua_document_element_instancer = new LuaDocumentElementInstancer();
		lua_event_listener_instancer = new LuaEventListenerInstancer();
		Rml::Factory::RegisterElementInstancer("body", lua_document_element_instancer);
		Rml::Factory::RegisterEventListenerInstancer(lua_event_listener_instancer);
	}

	void SolPlugin::OnShutdown()
	{
		g_L = nullptr;

		delete this;
	}

	void SolPlugin::RegisterTypes()
	{
		RMLUI_ASSERT(g_L);
		sol::state* L = g_L.get();

		RmlSolInitTypes(L);
	}

	sol::state* SolPlugin::GetLuaState()
	{
		return g_L.get();
	}

	void SolPlugin::RebindLuaState(NN::Ref<sol::state> lua_state)
	{
		g_L = lua_state;
		RegisterTypes();
	}
}
