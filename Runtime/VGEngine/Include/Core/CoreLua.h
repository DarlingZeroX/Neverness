#pragma once
#include "CoreTypes.h"
#include "../EngineConfig.h"
#include "../Lua/sol2/sol.hpp"

namespace VisionGal
{

	struct VG_ENGINE_API CoreLua
	{
		static void Initialize();
		static sol::state* GetCoreLuaState();
	};


}
