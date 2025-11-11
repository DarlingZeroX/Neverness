#pragma once
#include "Header.h"
#include <RmlUi/Core/Types.h>
#include <sol/state.hpp>
#include <sol/function.hpp>

namespace RmlSol {
	namespace Interpreter {
		/**
		@return The lua_State that the Interpreter created in Interpreter::Startup()
		@remark This class lacks a SetLuaState for a reason. If you have to use a seperate Lua binding and want to keep the types
		from RmlUi, then use this lua_State; it will already have all of the libraries loaded, and all of the types defined.
		Alternatively, you can initialise the plugin with your own Lua state if you need them defined in it. */
		RMLUISOL_API sol::state* GetLuaState();

		/** This function calls luaL_loadfile and then lua_pcall, reporting the errors (if any)
		@param[in] file Fully qualified file name to execute.
		@remark Somewhat misleading name if you are used to the Lua meaning of "load file". It behaves
		exactly as luaL_dofile does.            */
		RMLUISOL_API bool LoadFile(const Rml::String& file);
		/** Calls lua_dostring and reports the errors.
		@param[in] code String to execute
		@param[in] name Name for the code that will show up in the Log  */
		RMLUISOL_API bool DoString(const Rml::String& code, const Rml::String& name = "");

		RMLUISOL_API bool LoadFunction(sol::function& function, const Rml::String& code, const Rml::String& name = "");

	} 
} 
