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

#include "Log.h"
#include <RmlUi/Core/Log.h>

namespace RmlSol {

	void RmlLog::RegisterType(sol::state* lua, sol::table& rml)
	{
		rml.set_function("Message", [](int type, const std::string& msg)
			{
				Rml::Log::Type logType = static_cast<Rml::Log::Type>(type);
				return Rml::Log::Message(logType, "%s", msg);
			});

		sol::table log_type = rml["log_type"].get_or_create<sol::table>();
		log_type["always"] = Rml::Log::LT_ALWAYS;
		log_type["error"] = Rml::Log::LT_ERROR;
		log_type["warning"] = Rml::Log::LT_WARNING;
		log_type["info"] = Rml::Log::LT_INFO;
		log_type["debug"] = Rml::Log::LT_DEBUG;
	}
}
