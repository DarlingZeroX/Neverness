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

#include "ElementFormControlInput.h"
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Elements/ElementForm.h>
#include <RmlUi/Core/Elements/ElementFormControlInput.h>

namespace RmlSol {

	void RmlElementFormControlInput::RegisterType(sol::state* lua)
	{
		//lua->new_usertype<Rml::ElementFormControlInput>("RmlElementFormControlInput",
		//	sol::bases<Rml::ElementFormControl>(),
		//	sol::constructors<Rml::ElementFormControlInput(const Rml::String&)>(),
		//	"maxlength", sol::property(
		//		[](Rml::ElementFormControlInput& self) -> int { return self.GetAttribute<int>("maxlength", -1); },
		//		[](Rml::ElementFormControlInput& self, int maxlength)
		//		{
		//			self.SetAttribute("maxlength", maxlength);
		//		}
		//	));

		// Rml::Colourf
		lua->new_usertype<Rml::ElementFormControlInput>("RmlElementFormControlInput",
			sol::bases<Rml::ElementFormControl>(),
			sol::constructors<Rml::ElementFormControlInput(const Rml::String&)>(),
			"value", sol::property(
				[](Rml::ElementFormControlInput& self) -> std::string { return self.GetValue(); },
				[](Rml::ElementFormControlInput& self, const std::string& value) { self.SetValue(value); }
			),
			"isSubmitted", sol::property(
				[](Rml::ElementFormControlInput& self) -> bool { return self.IsSubmitted(); }
			),
			"Select", &Rml::ElementFormControlInput::Select,
			"SetSelectionRange", &Rml::ElementFormControlInput::SetSelectionRange,
			//"GetSelection", &Rml::ElementFormControlInput::SetSelectionRange,
			"SetCompositionRange", & Rml::ElementFormControlInput::SetCompositionRange,
			"checked", sol::property(
				[](Rml::ElementFormControlInput& self) -> bool {return self.HasAttribute("checked");},
				[](Rml::ElementFormControlInput& self, bool checked) -> void
				{
					if (checked)
						self.SetAttribute("checked", true);
					else
						self.RemoveAttribute("checked");
				}
			),
			"maxlength", sol::property(
				[](Rml::ElementFormControlInput& self) -> int { return self.GetAttribute<int>("maxlength", -1); },
				[](Rml::ElementFormControlInput& self, int maxlength)
				{
					self.SetAttribute("maxlength", maxlength);
				}
			),
			"size", sol::property(
				[](Rml::ElementFormControlInput& self) -> int { return self.GetAttribute<int>("size", 20); },
				[](Rml::ElementFormControlInput& self, int size)
				{
					self.SetAttribute("size", size);
				}
			),
			"max", sol::property(
				[](Rml::ElementFormControlInput& self) -> int { return self.GetAttribute<int>("max",100); },
				[](Rml::ElementFormControlInput& self, int max)
				{
					self.SetAttribute("max", max);
				}
			),
			"min", sol::property(
				[](Rml::ElementFormControlInput& self) -> int { return self.GetAttribute<int>("min", 0); },
				[](Rml::ElementFormControlInput& self, int min)
				{
					self.SetAttribute("min", min);
				}
			),
			"step", sol::property(
				[](Rml::ElementFormControlInput& self) -> int { return self.GetAttribute<int>("step", 1); },
				[](Rml::ElementFormControlInput& self, int setp)
				{
					self.SetAttribute("setp", setp);
				}
			)
		);

	}
}
