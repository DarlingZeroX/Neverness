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

#include "Lua/CoreLuaInterface.h"
#include "VGCore/Include/Core/Application.h"
#include "VGCore/Include/Core/Input.h"

namespace VisionGal
{
	namespace Lua
	{
		void CoreTypesLuaInterface::Initialize(sol::state& lua)
		{
			lua.new_usertype<int2>("int2",
				sol::constructors<int2(int, int)>(),
				"x", &int2::x,
				"y", &int2::y
			);

			lua.new_usertype<float2>("float2",
				sol::constructors<float2(float, float)>(),
				"x", &float2::x,
				"y", &float2::y,
				"r", &float2::x,
				"g", &float2::y
			);

			lua.new_usertype<float3>("float3",
				sol::constructors<float3(float, float, float)>(),
				"x", &float3::x,
				"y", &float3::y,
				"z", &float3::z,
				"r", &float3::x,
				"g", &float3::y,
				"b", &float3::z
			);

			lua.new_usertype<float4>("float4",
				sol::constructors<float4(float,float,float,float)>(),
				"x", &float4::x,
				"y", &float4::y,
				"z", &float4::z,
				"w", &float4::w,
				"r", &float4::x,
				"g", &float4::y,
				"b", &float4::z,
				"a", &float4::w
			);

			lua.new_usertype<quaternion>("quaternion",
				sol::constructors<quaternion(float, float, float, float)>(),
				"x", &quaternion::x,
				"y", &quaternion::y,
				"z", &quaternion::z,
				"w", &quaternion::w
			);
		}

		void ApplicationLuaInterface::ApplicationQuit()
		{
			Application::Quit();
		}

		void ApplicationLuaInterface::ApplicationOpenURL(const std::string& url)
		{
			Application::OpenURL(url);
		}

		void ApplicationLuaInterface::Initialize(sol::state& lua)
		{
			sol::table game = lua.create_named_table("Application");
			game.set_function("Quit", ApplicationQuit);
			game.set_function("OpenURL", ApplicationOpenURL);
		}

		int GameBindString()
		{
			return 0;
		}

		void DataBindingLuaInterface::Initialize(sol::state& lua)
		{
			//	luabridge::getGlobalNamespace(L)
			//		.beginNamespace("Game")
			//		.addFunction("BindString", GameBindString)  // 注册 C 风格函数
			//		.endNamespace();
		}

		void InputLuaInterface::Initialize(sol::state& lua)
		{
			sol::table game = lua.create_named_table("Input");
			game.set_function("GetMouseButtonDown", Input::GetMouseButtonDown);
			game.set_function("GetMouseButtonUp", Input::GetMouseButtonUp);
			game.set_function("GetMouseButtonHeld", Input::GetMouseButtonHeld);
			game.set_function("GetKeyName", Input::GetKeyName);
			game.set_function("GetKeyNameDown", Input::GetKeyNameDown);
			game.set_function("GetKeyNameUp", Input::GetKeyNameUp);
		}
	}

	void CoreLuaInterface::Initialize(sol::state& L)
	{
		Lua::CoreTypesLuaInterface::Initialize(L);
		Lua::InputLuaInterface::Initialize(L);
		Lua::ApplicationLuaInterface::Initialize(L);
	}
}


