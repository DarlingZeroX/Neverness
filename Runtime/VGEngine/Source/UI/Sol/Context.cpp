#include "Context.h"
#include <RmlUi/Core/Context.h>
#include "UI/UISystem.h"

namespace RmlSol {

	void RmlContext::RegisterType(sol::state* lua)
	{
		// Rml::Context
		lua->new_usertype<Rml::Context>("RmlContext",
			//sol::constructors<Rml::Context()>(),

			"LoadDocument", [](Rml::Context& self, const std::string& path)
			{
				Rml::ElementDocument* doc = self.LoadDocument(path);
				VisionGal::UISystem::Get()->OnScriptOpenDocument(doc);
				return doc;
			}
		);
	}
}
