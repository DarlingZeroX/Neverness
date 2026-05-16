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
#include <RmlUi/Core/Core.h>
#include "Sol/Sol.h"

namespace RmlSol {
	void Initialise()
	{
		::RmlSol::Initialise(nullptr);
	}

	void Initialise(NN::Ref<sol::state> lua_state)
	{
		::Rml::RegisterPlugin(new SolPlugin(lua_state));
	}

} 