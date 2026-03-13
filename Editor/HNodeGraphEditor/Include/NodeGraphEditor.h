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
#include "NodeGraphWindow.h"

namespace Horizon::NodeGraph
{
	class H_NODE_GRAPH_EDITOR_API NodeGraphEditor
	{
	public:
		NodeGraphEditor();
		~NodeGraphEditor();

		void SetContext();
		static void ResetContext();
		void DrawNodeGraphWindow(NodeGraphWindow& window);
	private:
		void Initialize();
	private:
		ax::NodeEditor::EditorContext* m_EditorContext;
	};

}