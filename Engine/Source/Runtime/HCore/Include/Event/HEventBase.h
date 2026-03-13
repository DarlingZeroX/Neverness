// Copyright (c) 2025 梦旅缘心
// This file is part of VisionGal and is licensed under the MIT License.
// See the LICENSE file in the project root for details.

#pragma once
#include "../CoreModuleDefinitions.h"
#include <string>
#include <functional>

namespace Horizon::Events
{
	enum class GlobalEventCategory
	{
		None,
		Application,
		//Input,
		//Keyboard,
		//Mouse,
		//MouseButton,
		Pipline,
		Scene,
		Editor,
		Window,
		Drop,
	};

	struct HEvent
	{
		//HEvent() = default;
		//HEvent(const HEvent&) = default;
		//HEvent& operator=(const HEvent&) { return *this; }
		virtual ~HEvent() = default;

		bool Handled = false;

		virtual std::uint32_t GetEventType() const = 0;
		virtual const char* GetName() const = 0;
		virtual GlobalEventCategory GetCategoryFlags() const = 0;
		virtual std::string ToString() const { return GetName(); }
	};

	template<typename T>
	static  T GetEventTypeAs(const HEvent* evt) { return static_cast<T>(evt->GetEventType()); }

	struct HEventDesc
	{
		bool synchronizing;
		//IEventHander* handler;
	};

#define GLOBAL_EVENT_CLASS_TYPE(category, type) static category GetStaticType() { return category::type; }\
								virtual std::uint32_t GetEventType() const override { return (std::uint32_t)GetStaticType(); }\
								virtual const char* GetName() const override { return #type; }

#define EVENT_CLASS_GLOBAL_CATEGORY(category) virtual GlobalEventCategory GetCategoryFlags() const override { return GlobalEventCategory::category; }
}
