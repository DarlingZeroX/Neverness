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
#include "IncludeHeader.h"

namespace RmlSol {

	struct RmlRmlUI
	{
		static void RegisterType(sol::state* lua, sol::table& rml);

		static void CreateKeyIdentifierTable(sol::table& rml);
		static void CreateKeyModifierTable(sol::table& rml);
	};


}
