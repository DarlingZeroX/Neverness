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

#include "LuaDocument.h"
#include <RmlUi/Core/Stream.h>
#include <Sol/Interpreter.h>

namespace RmlSol {

	LuaDocument::LuaDocument(const Rml::String& tag) : ElementDocument(tag) {}

	void LuaDocument::LoadInlineScript(const Rml::String& context, const Rml::String& source_path, int source_line)
	{
		Rml::String buffer;
		buffer += "--";
		buffer += source_path;
		buffer += ":";
		buffer += Rml::ToString(source_line);
		buffer += "\n";
		buffer += context;

		// 这里减去2是因为buffer贾玲源代码的第一行是注释，占用了一行
		Interpreter::DoStringErrorTrack(buffer, buffer, source_path, source_line - 2);
	}

	void LuaDocument::LoadExternalScript(const Rml::String& source_path)
	{
		Interpreter::LoadFile(source_path);
	}
}
