#include "LuaEventListener.h"
#include <RmlUi/Core/Element.h>
#include <UI/Sol/Interpreter.h>

namespace RmlSol {
	typedef Rml::ElementDocument Document;

	LuaEventListener::LuaEventListener(const Rml::String& code, Rml::Element* element) : EventListener()
	{
		// compose function
		Rml::String function = "return function (event,element,document) ";
		function.append(code);
		function.append(" end");

		attached = element;
		if (element)
			owner_document = element->GetOwnerDocument();
		else
			owner_document = nullptr;
		strFunc = function;
		m_Code = code;
	}

	// if it is passed in a Lua function
	LuaEventListener::LuaEventListener(lua_State* L, int narg, Rml::Element* element)
	{

	}

	LuaEventListener::~LuaEventListener()
	{

	}

	void LuaEventListener::OnDetach(Rml::Element* /*element*/)
	{
		// We consider this listener owned by its element, so we must delete ourselves when
		// we detach (probably because element was removed).
		delete this;
	}

	/// Process the incoming Event
	void LuaEventListener::ProcessEvent(Rml::Event& event)
	{
		// not sure if this is the right place to do this, but if the element we are attached to isn't a document, then
		// the 'owner_document' variable will be nullptr, because element->ower_document hasn't been set on the construction. We should
		// correct that
		if (!owner_document && attached)
			owner_document = attached->GetOwnerDocument();

		try {
			if (Interpreter::LoadFunction(m_Function, strFunc, m_Code))
			{
				auto result = m_Function(&event, attached, owner_document);
				if (!result.valid())
				{
					sol::error err = result;
					H_LOG_ERROR(err.what());
					Rml::Log::Message(Rml::Log::LT_WARNING, "%s", err.what());
				}
			}
		}
		catch (const sol::error& e) {
			H_LOG_ERROR("Error: %s", e.what());
		}

	}

}
