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

#pragma once
#include <RmlUi/Core/EventListenerInstancer.h>

namespace RmlSol {

	class LuaEventListenerInstancer : public ::Rml::EventListenerInstancer {
	public:
		/// Instance an event listener object.
		/// @param value Value of the event.
		/// @param element Element that triggers the events.
		Rml::EventListener* InstanceEventListener(const Rml::String& value, Rml::Element* element) override;
	};
}
