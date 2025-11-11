#include "SolPlugin.h"
#include <RmlUi/Core/Core.h>
#include "UI/Sol/Sol.h"

namespace RmlSol {
	void Initialise()
	{
		::RmlSol::Initialise(nullptr);
	}

	void Initialise(Ref<sol::state> lua_state)
	{
		::Rml::RegisterPlugin(new SolPlugin(lua_state));
	}

} 