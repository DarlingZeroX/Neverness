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

#include "Element.h"
#include "UI/Sol/Utilities.h"
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Elements/ElementForm.h>
#include <RmlUi/Core/Elements/ElementFormControl.h>
#include <RmlUi/Core/Elements/ElementFormControlInput.h>
#include <RmlUi/Core/Elements/ElementFormControlSelect.h>
#include <RmlUi/Core/Elements/ElementFormControlTextArea.h>
#include <RmlUi/Core/Elements/ElementTabSet.h>

namespace RmlSol {

	struct RmlElementAttributesProxy {
		Rml::Element* owner;
	};

	struct RmlElementChildNodesProxy {
		Rml::Element* owner;
	};

	struct RmlElementStyleProxy {
		Rml::Element* owner;
	};

	void RmlElement::RegisterType(sol::state* lua)
	{
		//Rml::Element ele;
		//ele.GetStyle()->

		// Rml::Element
		lua->new_usertype<Rml::Element>("RmlElement",
			sol::constructors<Rml::Element(const Rml::String&)>(),
			"checked", sol::property(
				[](Rml::Element& self) -> bool {return self.HasAttribute("checked"); },
				[](Rml::Element& self, bool checked) -> void
				{
					if (checked)
						self.SetAttribute("checked", true);
					else
						self.RemoveAttribute("checked");
				}
			),
			"AsForm", [](Rml::Element& self) ->Rml::ElementForm*
			{
				return  dynamic_cast<Rml::ElementForm*>(&self);
			},
			"AsFormControl", [](Rml::Element& self) ->Rml::ElementFormControl*
			{
				return  dynamic_cast<Rml::ElementFormControl*>(&self);
			},
			"AsFormControlInput", [](Rml::Element& self) ->Rml::ElementFormControlInput*
			{
				return  dynamic_cast<Rml::ElementFormControlInput*>(&self);
			},
			"AsFormControlSelect", [](Rml::Element& self) ->Rml::ElementFormControlSelect*
			{
				return  dynamic_cast<Rml::ElementFormControlSelect*>(&self);
			},
			"AsFormControlTextArea", [](Rml::Element& self) ->Rml::ElementFormControlTextArea*
			{
				return  dynamic_cast<Rml::ElementFormControlTextArea*>(&self);
			},
			"AsTabSet", [](Rml::Element& self) ->Rml::ElementTabSet*
			{
				return  dynamic_cast<Rml::ElementTabSet*>(&self);
			},
			
			
			"AppendChild", [](Rml::Element& obj, Rml::Element* ptr) -> Rml::Element*
			{
				if (ptr)
				{
					Rml::ElementPtr element(ptr);
					Rml::Element* child = obj.AppendChild(std::move(element));
					return child;
				}
				else
				{
					Rml::Log::Message(Rml::Log::LT_WARNING, "Could not append child to element '%s', as the child was null. Was it already moved from?",
						obj.GetAddress().c_str());
					return nullptr;
				}
			},
			"Blur", & Rml::Element::Blur,
			"Click", & Rml::Element::Click,
			//"DispatchEvent", & Rml::Element::Focus,
			"Focus", & Rml::Element::Focus,
			"GetAttribute", [](Rml::Element& self, const std::string& key, sol::this_state lua)
			{
				const Rml::Variant* param = self.GetAttribute(key);
				return PushVariant(lua, param);
			},
			"GetElementById", & Rml::Element::GetElementById,
			//"GetElementsByTagName", & Rml::Element::GetElementById,
			"QuerySelector", &Rml::Element::QuerySelector,
			//"QuerySelectorAll", &Rml::Element::QuerySelector,
			"Matches", &Rml::Element::Matches,
			"HasAttribute", &Rml::Element::HasAttribute,
			"HasChildNodes", &Rml::Element::HasChildNodes,
			//"InsertBefore", &Rml::Element::IsClassSet,
			"IsClassSet", &Rml::Element::IsClassSet,
			"RemoveAttribute", &Rml::Element::RemoveAttribute,
			"RemoveChild", [](Rml::Element& obj, Rml::Element* element)
			{
				bool result = static_cast<bool>( obj.RemoveChild(element) );
				return result;
			},
			//"ReplaceChild", [](Rml::Element& obj, bool align) {obj.ScrollIntoView(align); },
			"ScrollIntoView", [](Rml::Element& obj, bool align) {obj.ScrollIntoView(align); },
			"SetAttribute", &Rml::Element::SetAttribute<Rml::String>,
			"SetClass", &Rml::Element::SetClass,

			// Setter
			"attributes", sol::property(
				[](Rml::Element& self) -> RmlElementAttributesProxy
				{
					RmlElementAttributesProxy proxy;
					proxy.owner = &self;
					return proxy;
				}
			),
			"childNodes", sol::property(
				[](Rml::Element& self) -> RmlElementChildNodesProxy
				{
					RmlElementChildNodesProxy proxy;
					proxy.owner = &self;
					return proxy;
				}
			),
			"className", sol::property(
				[](Rml::Element& self) -> const std::string& { return self.GetClassNames(); },
				[](Rml::Element& self, const std::string& value) { self.SetClassNames(value); }
			),
			"clientLeft", sol::property(
				[](Rml::Element& self)-> float { return self.GetClientLeft(); }
			),
			"clientHeight", sol::property(
				[](Rml::Element& self)-> float { return self.GetClientHeight(); }
			),
			"clientTop", sol::property(
				[](Rml::Element& self)-> float { return self.GetClientTop(); }
			),
			"clientWidth", sol::property(
				[](Rml::Element& self)-> float { return self.GetClientWidth(); }
			),
			"firstChild", sol::property(
				[](Rml::Element& self)-> Rml::Element* { return self.GetFirstChild(); }
			),
			"id", sol::property(
				[](Rml::Element& self) -> const std::string& { return self.GetId(); },
				[](Rml::Element& self, const std::string& value) { self.SetId(value); }
			),
			"innerRml", sol::property(
				[](Rml::Element& self) -> const std::string& { return self.GetInnerRML(); },
				[](Rml::Element& self, const std::string& value) { self.SetInnerRML(value); }
			),
			"lastChild", sol::property(
				[](Rml::Element& self)-> Rml::Element* { return self.GetLastChild(); }
			),
			"nextSibling", sol::property(
				[](Rml::Element& self)-> Rml::Element* { return self.GetNextSibling(); }
			),
			"offsetHeight", sol::property(
				[](Rml::Element& self)-> float { return self.GetOffsetHeight(); }
			),
			"offsetLeft", sol::property(
				[](Rml::Element& self)-> float { return self.GetOffsetLeft(); }
			),
			"offsetParent", sol::property(
				[](Rml::Element& self)-> Rml::Element* { return self.GetOffsetParent(); }
			),
			"offsetTop", sol::property(
				[](Rml::Element& self)-> float { return self.GetOffsetTop(); }
			),
			"offsetWidth", sol::property(
				[](Rml::Element& self)-> float { return self.GetOffsetWidth(); }
			),
			"ownerDocument", sol::property(
				[](Rml::Element& self)-> Rml::ElementDocument* { return self.GetOwnerDocument(); }
			),
			"parentNode", sol::property(
				[](Rml::Element& self)-> Rml::Element* { return self.GetParentNode(); }
			),
			"previousSibling", sol::property(
				[](Rml::Element& self)-> Rml::Element* { return self.GetPreviousSibling(); }
			),
			"scrollHeight", sol::property(
				[](Rml::Element& self) -> float { return self.GetScrollHeight(); }
			),
			"scrollLeft", sol::property(
				[](Rml::Element& self) -> float { return self.GetScrollLeft(); },
				[](Rml::Element& self, float value) { self.SetScrollLeft(value); }
			),
			"scrollTop", sol::property(
				[](Rml::Element& self) -> float { return self.GetScrollTop(); },
				[](Rml::Element& self, float value) { self.SetScrollTop(value); }
			),
			"scrollWidth", sol::property(
				[](Rml::Element& self) -> float { return self.GetScrollWidth(); }
			),
			"style", sol::property(
				[](Rml::Element& self) -> RmlElementStyleProxy
				{
					RmlElementStyleProxy proxy;
					proxy.owner = &self;
					return proxy;
				}
			),
			"tagName", sol::property(
				[](Rml::Element& self) -> const std::string& { return self.GetTagName(); }
			)
		);

		lua->new_usertype<RmlElementAttributesProxy>("RmlEventParametersProxy",
			sol::meta_function::index, [](RmlElementAttributesProxy& self, const std::string& key, sol::this_state lua) {
				auto it = self.owner->GetAttributes().find(key);
				const Rml::Variant* param = (it == self.owner->GetAttributes().end() ? nullptr : &it->second);

				return PushVariant(lua, param);
			});

		lua->new_usertype<RmlElementChildNodesProxy>("RmlEventParametersProxy",
			sol::meta_function::index, [](RmlElementChildNodesProxy& self, int key) {
				return self.owner->GetChild(key);
			},
			sol::meta_function::length, [](RmlElementChildNodesProxy& self) {
				return self.owner->GetNumChildren();
			}
		);

		lua->new_usertype<RmlElementStyleProxy>("RmlElementStyleProxy",
			sol::meta_function::index, [](RmlElementStyleProxy& self, const std::string& key) {
				const Rml::Property* prop = self.owner->GetProperty(key);

				return prop->ToString();
			},
			sol::meta_function::new_index, [](RmlElementStyleProxy& self, const std::string& key, const std::string& value) {
				bool result = true;
				if (value == "")
				{
					self.owner->RemoveProperty(key);
				}
				else
				{
					result = self.owner->SetProperty(key, value);
				}

				return result;
			}
		);
	}
}
