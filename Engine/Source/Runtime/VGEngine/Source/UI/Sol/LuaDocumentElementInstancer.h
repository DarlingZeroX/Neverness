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
#include <RmlUi/Core/ElementInstancer.h>

namespace RmlSol {

	class LuaDocumentElementInstancer : public ::Rml::ElementInstancer {
		/// Instances an element given the tag name and attributes.
		/// @param[in] parent The element the new element is destined to be parented to.
		/// @param[in] tag The tag of the element to instance.
		/// @param[in] attributes Dictionary of attributes.
		Rml::ElementPtr InstanceElement(Rml::Element* parent, const Rml::String& tag, const Rml::XMLAttributes& attributes) override;
		/// Releases an element instanced by this instancer.
		/// @param[in] element The element to release.
		void ReleaseElement(Rml::Element* element) override;
	};
}
