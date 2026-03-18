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

#include <VGImgui/Include/ImNodeEditor/imgui_node_editor.h>
#include <HNGRuntimeCore/Interface/RuntimeCore.h>
//namespace IMNE = ax::NodeEditor;

namespace Horizon::NodeGraphEditor
{
	using EditorNodeID = ax::NodeEditor::NodeId;
	using EditorLinkID = ax::NodeEditor::LinkId;
	using EditorPinID = ax::NodeEditor::PinId;

	struct EditorPin
	{
		ax::NodeEditor::PinId id;

		std::string name;
		NodeGraphRuntime::SlotType type;

		bool isInput;

		// UI
		ImColor color;

		// 编译用
		uint32_t runtimeIndex = 0;
	};

	struct EditorNode
	{
		EditorNodeID id;

		NodeGraphRuntime::NodeType type;
		std::string name;

		std::vector<EditorPin> inputs;
		std::vector<EditorPin> outputs;

		ImVec2 position;

		// 自定义数据（用于 Dialogue 等）
		std::unordered_map<std::string, std::string> properties;
	};

	struct EditorLink
	{
		EditorLinkID id;
		
		EditorPinID startPinId;
		EditorPinID endPinId;
	};
}