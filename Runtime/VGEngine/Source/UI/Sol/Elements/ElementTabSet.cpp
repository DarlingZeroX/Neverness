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

#include "ElementTabSet.h"
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Elements/ElementForm.h>
#include <RmlUi/Core/Elements/ElementTabSet.h>

namespace RmlSol {

	void RmlElementTabSet::RegisterType(sol::state* lua)
	{
		// Rml::Colourf
		lua->new_usertype<Rml::ElementTabSet>("RmlElementTabSet",
			sol::bases<Rml::Element>(),
			sol::constructors<Rml::ElementTabSet(const Rml::String&)>(),
			"SetPanel", [](Rml::ElementTabSet& self, int index, const std::string& rml)
			{
				self.SetPanel(index, rml);
			},
			"SetTab", [](Rml::ElementTabSet& self, int index, const std::string& rml)
			{
				self.SetTab(index, rml);
			},
			"RemoveTab", & Rml::ElementTabSet::RemoveTab,
			"activeTab", sol::property(
				[](Rml::ElementTabSet& self) -> int { return self.GetActiveTab(); },
				[](Rml::ElementTabSet& self, int value) { self.SetActiveTab(value); }
			),
			"numTabs", sol::property(
				[](Rml::ElementTabSet& self) -> int { return self.GetNumTabs(); }
			)
		);

	}
}
