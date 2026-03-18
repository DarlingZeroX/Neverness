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
#include <string>
#include "../HNGEditorCoreConfig.h"
#include "EditorCore.h"

namespace Horizon::NodeGraphEditor
{
	class HNG_EDITOR_CORE_API IMNEEditorContext
	{
	public:
		IMNEEditorContext();
		IMNEEditorContext(const ax::NodeEditor::Config& config);
		~IMNEEditorContext();

		void SetContext();
		static void ResetContext();

		ax::NodeEditor::EditorContext* m_EditorContext;
	};
}