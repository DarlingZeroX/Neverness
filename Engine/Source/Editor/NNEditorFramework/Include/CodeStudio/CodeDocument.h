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
#include "../../Config.h"
#include "NNRuntimeCore/Include/Core/Core.h"
#include "NNRuntimeImGui/Include/Imgui/imgui.h"
#include <NNRuntimeImGui/Include/ImGuiColorTextEdit/TextEditor.h>

namespace VisionGal::Editor
{
	struct VG_EDITOR_FRAMEWORK_API CodeDocument
	{
		std::string        Name;   // Document title
		int         UID;        // Unique ID (necessary as we can change title)
		bool        IsOpen;       // Set when open (we keep an array of all available documents to simplify demo code!)
		bool        OpenPrev;   // Copy of Open from last update.
		bool        Dirty;      // Set when the document has been modified
		ImVec4      Color;      // An arbitrary variable associated to the document
		std::filesystem::file_time_type FileLastWriteTime;
		bool		FirstUseEver = true;
		bool		NeedFocus = false;

		std::string DocPath;

		ImGuiTextEditor::TextEditor TexEditor;

		CodeDocument();

		void Initialize();
		void Update();
		void DoOpen()       { IsOpen = true; }
		void DoForceClose() { IsOpen = false; Dirty = false; }
		void DoSave();

		void ReadLastWriteTime();
		bool OpenFile(const VGPath& path);
	};
}
