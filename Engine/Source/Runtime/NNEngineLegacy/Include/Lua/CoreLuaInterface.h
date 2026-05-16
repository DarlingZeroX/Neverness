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
#include <sol/state.hpp>

namespace NN::Runtime
{
	namespace Lua
	{
		struct CoreTypesLuaInterface
		{
			static void Initialize(sol::state& L);
		};

		struct ApplicationLuaInterface
		{
			static void ApplicationQuit();
			static void ApplicationOpenURL(const std::string& url);

			static void Initialize(sol::state& L);
		};

		struct DataBindingLuaInterface
		{
			static void Initialize(sol::state& L);
		};

		struct InputLuaInterface
		{
			static void Initialize(sol::state& L);
		};
	}

	struct CoreLuaInterface
	{
		static void Initialize(sol::state& L);
	};
}