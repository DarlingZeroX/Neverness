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
/*
    This class is an ElementDocument that overrides the LoadInlineScript and LoadExternalScript function
*/
#include <RmlUi/Core/ElementDocument.h>

namespace RmlSol {

	class LuaDocument : public ::Rml::ElementDocument {
	public:
		LuaDocument(const Rml::String& tag);
		void LoadInlineScript(const Rml::String& content, const Rml::String& source_path, int source_line) override;
		void LoadExternalScript(const Rml::String& source_path) override;
	};

}
