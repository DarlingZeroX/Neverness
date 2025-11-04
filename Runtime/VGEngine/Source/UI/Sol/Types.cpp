#include "Types.h"
#include <RmlUi/Core/Event.h>
#include <RmlUi/Core/ElementDocument.h>
#include "UI/UISystem.h"
#include "UI/Sol/Utilities.h"

#include "RmlUi.h"
#include "Colourf.h"
#include "Vector2i.h"
#include "Vector2f.h"
#include "Log.h"
#include "Context.h"
#include "Document.h"
#include "Element.h"
#include "ElementText.h"
#include "Event.h"

#include "Elements/ElementForm.h"
#include "Elements/ElementFormControl.h"
#include "Elements/ElementFormControlInput.h"
#include "Elements/ElementFormControlSelect.h"
#include "Elements/ElementFormControlTextArea.h"
#include "Elements/ElementTabSet.h"

namespace RmlSol {

	void RmlSolInitTypes(sol::state* lua)
	{
		sol::table rmlTable = lua->create_named_table("rmlui");

		RmlVector2i::RegisterType(lua);
		RmlVector2f::RegisterType(lua);
		RmlColourf::RegisterType(lua);
		RmlLog::RegisterType(lua, rmlTable);

		RmlElement::RegisterType(lua);
		RmlElementDocument::RegisterType(lua);
		RmlElementText::RegisterType(lua);
		RmlEvent::RegisterType(lua);
		RmlContext::RegisterType(lua);
		RmlRmlUI::RegisterType(lua, rmlTable);

		RmlElementForm::RegisterType(lua);
		RmlElementFormControl::RegisterType(lua);
		RmlElementFormControlInput::RegisterType(lua);
		RmlElementFormControlSelect::RegisterType(lua);
		RmlElementFormControlTextArea::RegisterType(lua);
		RmlElementTabSet::RegisterType(lua);
	}
}
