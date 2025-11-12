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

#include "ElementFormControl.h"
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Elements/ElementForm.h>
#include <RmlUi/Core/Elements/ElementFormControl.h>

namespace RmlSol {

	void RmlElementFormControl::RegisterType(sol::state* lua)
	{
		// Rml::Colourf
		lua->new_usertype<Rml::ElementFormControl>("RmlElementFormControl",
			sol::bases<Rml::Element>(),
			sol::no_constructor,
			"disabled", sol::property(
				[](Rml::ElementFormControl& self) -> bool { return self.IsDisabled(); },
				[](Rml::ElementFormControl& self, bool value) { self.SetDisabled(value); }
			),
			"name", sol::property(
				[](Rml::ElementFormControl& self) -> std::string { return self.GetName(); },
				[](Rml::ElementFormControl& self, const std::string& value) { self.SetName(value); }
			),
			"value", sol::property(
				[](Rml::ElementFormControl& self) -> std::string { return self.GetValue(); },
				[](Rml::ElementFormControl& self, const std::string& value) { self.SetValue(value); }
			),
			"isSubmitted", sol::property(
				[](Rml::ElementFormControl& self) -> bool { return self.IsSubmitted(); }
			)
		);

	}
}
