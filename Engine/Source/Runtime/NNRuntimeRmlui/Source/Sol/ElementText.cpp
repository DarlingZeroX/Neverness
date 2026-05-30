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

#include "ElementText.h"
#include "UISystemLegacy.h"
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>

namespace RmlSol {

	void RmlElementText::RegisterType(sol::state* lua)
	{
		// Rml::ElementText
		lua->new_usertype<Rml::ElementText>("RmlElementText",
			"text", sol::property(
				[](Rml::ElementText& self) -> const std::string& { return self.GetText(); },
				[](Rml::ElementText& self, const std::string& value) { self.SetText(value); }
			)
		);
	}
}
