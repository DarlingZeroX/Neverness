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
#include <atomic>
#include <unordered_map>
#include "../HNGEditorCoreConfig.h"
#include "EditorCore.h"
#include "EditorGraph.h"

namespace Horizon::NodeGraphEditor
{
	// 工厂用于生成唯一 NodeId/PinId
	inline ax::NodeEditor::NodeId GenNodeId() {
		static std::atomic<int> counter{100};
		return ax::NodeEditor::NodeId(counter++);
	}
	inline ax::NodeEditor::PinId GenPinId() {
		static std::atomic<int> counter{1000};
		return ax::NodeEditor::PinId(counter++);
	}

	inline EditorNode CreateEntryNode() {
		EditorNode node;
		node.id = GenNodeId();
		node.type = NodeGraphRuntime::NodeType::Entry;
		node.name = "Entry";
		// 输出：Next
		EditorPin outPin;
		outPin.id = GenPinId();
		outPin.name = "Next";
		outPin.type = NodeGraphRuntime::SlotType::Exec;
		outPin.isInput = false;
		node.outputs.push_back(outPin);
		return node;
	}

	EditorNode CreateDialogueNode(const std::string& text = "") {
		EditorNode node;
		node.id = GenNodeId();
		node.type = NodeGraphRuntime::NodeType::Dialogue;
		node.name = "Dialogue";
		// 输入：In
		EditorPin inPin;
		inPin.id = GenPinId();
		inPin.name = "In";
		inPin.type = NodeGraphRuntime::SlotType::Exec;
		inPin.isInput = true;
		node.inputs.push_back(inPin);
		// 输出：Next
		EditorPin outPin;
		outPin.id = GenPinId();
		outPin.name = "Next";
		outPin.type = NodeGraphRuntime::SlotType::Exec;
		outPin.isInput = false;
		node.outputs.push_back(outPin);
		// 输出：Text（对白内容，由编译器填充 Value）
		EditorPin outText;
		outText.id = GenPinId();
		outText.name = "Text";
		outText.type = NodeGraphRuntime::SlotType::String;
		outText.isInput = false;
		node.outputs.push_back(outText);
		// 属性
		node.properties["text"] = text;
		return node;
	}

	inline EditorNode CreateBranchNode() {
		EditorNode node;
		node.id = GenNodeId();
		node.type = NodeGraphRuntime::NodeType::Branch;
		node.name = "Branch";
		// 输入：Exec
		EditorPin inExec;
		inExec.id = GenPinId();
		inExec.name = "In";
		inExec.type = NodeGraphRuntime::SlotType::Exec;
		inExec.isInput = true;
		node.inputs.push_back(inExec);
		// 输入：Bool
		EditorPin inCond;
		inCond.id = GenPinId();
		inCond.name = "Condition";
		inCond.type = NodeGraphRuntime::SlotType::Bool;
		inCond.isInput = true;
		node.inputs.push_back(inCond);
		// 输出：True
		EditorPin outTrue;
		outTrue.id = GenPinId();
		outTrue.name = "True";
		outTrue.type = NodeGraphRuntime::SlotType::Exec;
		outTrue.isInput = false;
		node.outputs.push_back(outTrue);
		// 输出：False
		EditorPin outFalse;
		outFalse.id = GenPinId();
		outFalse.name = "False";
		outFalse.type = NodeGraphRuntime::SlotType::Exec;
		outFalse.isInput = false;
		node.outputs.push_back(outFalse);
		return node;
	}
}