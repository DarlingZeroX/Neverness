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
#include "Config.h"
#include "HNGEditorCore/Include/EditorGraph.h"
#include <HNGRuntimeCore/Include/Core/RuntimeContext.h>
#include <HNGRuntimeCore/Include/Core/RuntimeGraph.h>
#include <HNGRuntimeCore/Include/Core/NodeRegistry.h>

namespace Horizon::NodeGraph
{
	class HNG_EDITOR_API HNodeGraphEditor
	{
	public:
		HNodeGraphEditor();
		~HNodeGraphEditor();

		Horizon::NodeGraphRuntime::RuntimeGraph RecompileIfDirty(NodeGraphEditor::EditorGraph& editor);
		void DrawNodeGraphWindow();
	private:
		void RunDemo();

		NodeGraphEditor::EditorGraph m_EditorGraph;
		Horizon::NodeGraphRuntime::RuntimeGraph m_RuntimeGraph;
		Horizon::NodeGraphRuntime::RuntimeContext m_RuntimeContext;
		Horizon::NodeGraphRuntime::NodeRegistry m_Registry;
		//ax::NodeEditor::EditorContext* m_EditorContext;
	};

}