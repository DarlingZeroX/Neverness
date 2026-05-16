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

#include "Context.h"
#include <RmlUi/Core/Context.h>
#include "UISystem.h"

namespace RmlSol {

	void RmlContext::RegisterType(sol::state* lua)
	{
		// Rml::Context
		lua->new_usertype<Rml::Context>("RmlContext",
			//sol::constructors<Rml::Context()>(),

			"LoadDocument", [](Rml::Context& self, const std::string& path)
			{
				Rml::ElementDocument* doc = self.LoadDocument(path);
				NN::Runtime::UISystem::Get()->OnScriptOpenDocument(doc);
				return doc;
			}
		);
	}
}
