#include "ElementText.h"
#include "UI/UISystem.h"
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
