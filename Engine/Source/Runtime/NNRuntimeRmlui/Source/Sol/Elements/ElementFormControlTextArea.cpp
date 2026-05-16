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

#include "ElementFormControlTextArea.h"
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Elements/ElementForm.h>
#include <RmlUi/Core/Elements/ElementFormControlTextArea.h>

namespace RmlSol {

	void RmlElementFormControlTextArea::RegisterType(sol::state* lua)
	{
		// Rml::Colourf
		lua->new_usertype<Rml::ElementFormControlTextArea>("RmlElementFormControlTextArea",
			sol::bases<Rml::ElementFormControl>(),
			sol::constructors<Rml::ElementFormControlTextArea(const Rml::String&)>(),
			"value", sol::property(
				[](Rml::ElementFormControlTextArea& self) -> std::string { return self.GetValue(); },
				[](Rml::ElementFormControlTextArea& self, const std::string& value) { self.SetValue(value); }
			),
			"cols", sol::property(
				[](Rml::ElementFormControlTextArea& self) -> int { return self.GetNumColumns(); },
				[](Rml::ElementFormControlTextArea& self, int value) { self.SetNumColumns(value); }
			),
			"maxlength", sol::property(
				[](Rml::ElementFormControlTextArea& self) -> int { return self.GetMaxLength(); },
				[](Rml::ElementFormControlTextArea& self, int value) { self.SetMaxLength(value); }
			),
			"rows", sol::property(
				[](Rml::ElementFormControlTextArea& self) -> int { return self.GetNumRows(); },
				[](Rml::ElementFormControlTextArea& self, int value) { self.SetNumRows(value); }
			),
			"wordwrap", sol::property(
				[](Rml::ElementFormControlTextArea& self) -> bool { return self.GetWordWrap(); },
				[](Rml::ElementFormControlTextArea& self, bool value) { self.SetWordWrap(value); }
			),
			"Select", &Rml::ElementFormControlTextArea::Select,
			"SetSelectionRange", &Rml::ElementFormControlTextArea::SetSelectionRange,
			"SetCompositionRange", &Rml::ElementFormControlTextArea::SetCompositionRange
			//"GetIntrinsicDimensions", &Rml::ElementFormControlTextArea::SetCompositionRange
		);

	}
}
