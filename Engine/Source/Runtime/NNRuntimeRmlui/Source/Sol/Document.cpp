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

#include "Document.h"
//#include "UISystemLegacy.h"
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>

namespace RmlSol {

	void RmlElementDocument::RegisterType(sol::state* lua)
	{
		// Rml::ElementDocument
		//lua->new_usertype<Rml::ElementDocument>("RmlElementDocument",
		//	sol::constructors<Rml::ElementDocument(const Rml::String&)>(),
		//	// 方法
		//	"PullToFront", &Rml::ElementDocument::PullToFront,
		//	"PushToBack", &Rml::ElementDocument::PushToBack,
		//	"Show", [](Rml::ElementDocument& self) {
		//		self.Show();
		//	},
		//	"Hide", &Rml::ElementDocument::Hide,
		//	"Close", [](Rml::ElementDocument& self) {
		//		//self.Close();
		//		//NN::Runtime::UISystemLegacy::Get()->OnScriptCloseDocument(&self);
		//	},
		//	"CreateElement", [](Rml::ElementDocument& self, const Rml::String& name) {
		//		Rml::ElementPtr* ele = new Rml::ElementPtr(self.CreateElement(name));
		//		return ele->get();
		//	},
		//	"AddUpdateCallback", [](Rml::ElementDocument& self, const sol::function& callback) {
		//		//auto uiDocument = NN::Runtime::UISystemLegacy::Get()->FindDocumentByElementDocument(&self);
		//		//if (uiDocument == nullptr)
		//		//{
		//		//	uiDocument = NN::Runtime::UISystemLegacy::Get()->OnScriptOpenDocument(&self);
		//		//}
		//		//uiDocument->AddUpdateCallback(callback);
		//	},
		//	"AddTimerCallback", [](Rml::ElementDocument& self, float interval, const sol::function& callback) {
		//		//auto uiDocument = NN::Runtime::UISystemLegacy::Get()->FindDocumentByElementDocument(&self);
		//		//if (uiDocument == nullptr)
		//		//{
		//		//	uiDocument = NN::Runtime::UISystemLegacy::Get()->OnScriptOpenDocument(&self);
		//		//}
		//		//uiDocument->AddTimerCallback(interval, callback);
		//	}, 
		//	// 属性
		//	"title", sol::property(
		//		[](Rml::ElementDocument& self) -> const std::string& { return self.GetTitle(); },
		//		[](Rml::ElementDocument& self, const std::string& value) { self.SetTitle(value); }
		//	),
		//	"context", sol::property(
		//		[](Rml::ElementDocument& self) -> Rml::Context*
		//		{
		//			return self.GetContext();
		//		}
		//	),
		//	"isVisible", sol::property(
		//		[](Rml::ElementDocument& self) -> bool { return self.IsVisible(); }
		//	),
		//	"isVisible", sol::property(
		//		[](Rml::ElementDocument& self) -> bool { return self.IsVisible(); }
		//	),
		//	"visibility", sol::property(
		//		[](Rml::ElementDocument& self) -> bool
		//		{
		//			Rml::Style::Visibility v = self.GetProperty(Rml::PropertyId::Visibility)->Get<Rml::Style::Visibility>();
		//			if (v == Rml::Style::Visibility::Visible)
		//				return true;
		//			return false;
		//		},
		//		[](Rml::ElementDocument& self, bool value)
		//		{
		//			if (value == true)
		//			{
		//				self.Hide();
		//			}
		//			else
		//			{
		//				self.SetProperty(Rml::PropertyId::Visibility, Rml::Property(Rml::Style::Visibility::Visible));
		//				// We should update the document now, so that the (un)focusing will get the correct visibility
		//				self.UpdateDocument();
		//			}
		//		}
		//	)
		//);
	}
}
