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

#include "ElementFormControlSelect.h"
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Elements/ElementForm.h>
#include <RmlUi/Core/Elements/ElementFormControlSelect.h>

namespace RmlSol {

	void RmlElementFormControlSelect::RegisterType(sol::state* lua)
	{
		// Rml::Colourf
		lua->new_usertype<Rml::ElementFormControlSelect>("RmlElementFormControlSelect",
			sol::bases<Rml::ElementFormControl>(),
			sol::constructors<Rml::ElementFormControlSelect(const Rml::String&)>(),
			"value", sol::property(
				[](Rml::ElementFormControlSelect& self) -> std::string { return self.GetValue(); },
				[](Rml::ElementFormControlSelect& self, const std::string& value) { self.SetValue(value); }
			),
			"selection", sol::property(
				[](Rml::ElementFormControlSelect& self) -> int { return self.GetSelection(); },
				[](Rml::ElementFormControlSelect& self, int value) { self.SetSelection(value); }
			),
			//"options", sol::property(
			//	[](Rml::ElementFormControl& self) -> bool { return self.IsDisabled(); },
			//	[](Rml::ElementFormControl& self, bool value) { self.SetDisabled(value); }
			//),
			"GetNumOptions", & Rml::ElementFormControlSelect::GetNumOptions,
			"Add", [](Rml::ElementFormControlSelect& self, const std::string& rml, const std::string& value, int before) -> int
			{
				return self.Add(rml, value, before);
			},
			"Remove", & Rml::ElementFormControlSelect::Remove,
			"RemoveAll", & Rml::ElementFormControlSelect::RemoveAll,
			"ShowSelectBox", & Rml::ElementFormControlSelect::ShowSelectBox,
			"HideSelectBox", & Rml::ElementFormControlSelect::HideSelectBox,
			"CancelSelectBox", & Rml::ElementFormControlSelect::CancelSelectBox,
			"IsSelectBoxVisible", & Rml::ElementFormControlSelect::IsSelectBoxVisible
		);

	}
}
