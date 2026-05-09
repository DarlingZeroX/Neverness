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

#include <memory>
#include <string>
#include "../VGGalEditorConfig.h"
#include "SequenceEditor.h"

namespace VisionGal::Editor
{
	class VG_GALGAME_EDITOR_API VisualGalEditor
	{
	public:
		VisualGalEditor();
		~VisualGalEditor();

		void Initialize();

		void DrawEditorWindow();

	private:
		VGScriptSequenceEditor m_ScriptSequence;
	};
}
