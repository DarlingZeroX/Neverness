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
#include "IMNEWrap.h"

namespace Horizon::NodeGraphRuntime
{
	struct RuntimeContext;
}

namespace Horizon::NodeGraphEditor
{
	namespace Runtime = Horizon::NodeGraphRuntime;

	struct EditorGraph
	{
		std::vector<EditorNode> nodes;
		std::vector<EditorLink> links;

		Ref<IMNEEditorContext> context = nullptr;

		bool dirty = true;

		EditorNode* FindNode(ax::NodeEditor::NodeId id) { return nullptr; }
		EditorPin* FindPin(ax::NodeEditor::PinId id){ return nullptr; }
	};

	HNG_EDITOR_CORE_API void HandleCreateLink(EditorGraph& graph);
	HNG_EDITOR_CORE_API void DrawEditorGraph(EditorGraph& graph, const Runtime::RuntimeContext* runtimeCtx = nullptr);
}