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
#include <RmlUi/Core/Plugin.h>
#include "IncludeHeader.h"
#include "LuaDocumentElementInstancer.h"
#include "LuaEventListenerInstancer.h"
#include "HCore/Interface/HConfig.h"

namespace RmlSol {

	//class LuaDocumentElementInstancer;
	//class LuaEventListenerInstancer;

	class RMLUISOL_API SolPlugin : public Rml::Plugin {
	public:
		SolPlugin(Ref<sol::state> lua_state);

		static sol::state* GetLuaState();

		static void RebindLuaState(Ref<sol::state> lua_state);

	private:
		int GetEventClasses() override;

		void OnInitialise() override;

		void OnShutdown() override;

		static void RegisterTypes();

		LuaDocumentElementInstancer* lua_document_element_instancer = nullptr;
		LuaEventListenerInstancer* lua_event_listener_instancer = nullptr;
		bool owns_lua_state = false;
	};

} 
