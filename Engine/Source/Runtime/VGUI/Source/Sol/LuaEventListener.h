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

#include <RmlUi/Core/EventListener.h>
#include <RmlUi/Core/ElementDocument.h>
#include <Sol/IncludeSol.h>

namespace RmlSol {

	class LuaEventListener : public ::Rml::EventListener {
	public:
		// The plan is to wrap the code in an anonymous function so that we can have named parameters to use,
		// rather than putting them in global variables
		LuaEventListener(const Rml::String& code, Rml::Element* element);

		// This is called from a Lua Element if in element:AddEventListener it passes a function in as the 2nd
		// parameter rather than a string. We don't wrap the function in an anonymous function, so the user
		// should take care to have the proper order. The order is event,element,document.
		// narg is the position on the stack
		LuaEventListener(lua_State* L, int narg, Rml::Element* element);

		virtual ~LuaEventListener();

		// Deletes itself, which also unreferences the Lua function.
		void OnDetach(Rml::Element* element) override;

		// Calls the associated Lua function.
		void ProcessEvent(Rml::Event& event) override;

	private:
		// the lua-side function to call when ProcessEvent is called
		int luaFuncRef = -1;

		Rml::Element* attached = nullptr;
		Rml::ElementDocument* owner_document = nullptr;
		Rml::String strFunc; // for debugging purposes
		Rml::String m_Code;

		sol::function m_Function;
	};

}