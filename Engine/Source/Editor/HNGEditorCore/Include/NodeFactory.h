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
#include <HNGRuntimeCore/Include/Core/NodeMeta.h>

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

	// ------------------------------------------------------------
	// CreateNodeFromMeta：EditorNode 的唯一创建入口（数据驱动）
	//
	// 设计目标：
	// - Editor 不再手写 inputs/outputs pins（避免重复、易错、难维护）
	// - 节点结构完全由 Runtime 的 NodeMeta 描述（真正“蓝图编辑器”）
	//
	// 行为说明：
	// - 自动生成：
	//   - inputs / outputs
	//   - pin name
	//   - slot type
	//   - isInput
	// - pins 的 id 由 GenPinId() 生成，node.id 由 GenNodeId() 生成
	//
	// 约束：
	// - meta.inputs / meta.outputs 各自代表输入/输出 pin 列表；
	//   SlotMeta::isInput 字段在这里作为一致性校验来源（不依赖它决定方向）。
	// ------------------------------------------------------------
	inline EditorNode CreateNodeFromMeta(const Horizon::NodeGraphRuntime::NodeMeta& meta)
	{
		EditorNode node;
		node.id = GenNodeId();
		node.type = meta.type;
		node.name = meta.name;

		for (const auto& in : meta.inputs)
		{
			EditorPin pin;
			pin.id = GenPinId();
			pin.name = in.name;
			pin.type = in.type;
			pin.isInput = true;
			node.inputs.push_back(std::move(pin));
		}

		for (const auto& out : meta.outputs)
		{
			EditorPin pin;
			pin.id = GenPinId();
			pin.name = out.name;
			pin.type = out.type;
			pin.isInput = false;
			node.outputs.push_back(std::move(pin));
		}

		return node;
	}
}