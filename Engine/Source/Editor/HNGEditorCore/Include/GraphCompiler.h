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
#include "EditorGraph.h"

#include <HNGRuntimeCore/Include/Core/RuntimeGraph.h>
//////////////////////
#include "GraphCompiler.h"
#include <HNGRuntimeCore/Include/Core/Nodes.h>
#include <HNGRuntimeCore/Include/Core/NodeRegistry.h>
#include <unordered_map>
// hash/== for ax::NodeEditor::NodeId
#include <functional>

namespace ax {
	namespace NodeEditor {
		struct NodeId;
		struct PinId;
	}
}
namespace std {
	template<> struct hash<ax::NodeEditor::NodeId> {
		size_t operator()(const ax::NodeEditor::NodeId& id) const noexcept {
			// NodeId 必须能转换为整数（如 .Get()），否则请修改此处
			return std::hash<int>()(id.Get());
		}
	};

	// 让 EditorPinID(ax::NodeEditor::PinId) 可作为 unordered_map key
	template<> struct hash<ax::NodeEditor::PinId> {
		size_t operator()(const ax::NodeEditor::PinId& id) const noexcept {
			// PinId 必须能转换为整数（如 .Get()），否则请修改此处
			return std::hash<int>()(id.Get());
		}
	};
}

namespace Horizon::NodeGraphEditor
{
	NodeGraphRuntime::RuntimeGraph Compile(const EditorGraph& editor, const NodeGraphRuntime::NodeRegistry& registry)
	{
		using namespace NodeGraphRuntime;
		RuntimeGraph g;
		std::unordered_map<ax::NodeEditor::NodeId, NODE_ID> nodeIdMap;

		// ------------------------------
		// Pin 查找表（性能关键）
		// 目的：把 “PinId -> (nodeId, slotIndex, isOutput)” 的信息预先建立起来，后续 Link 转换 O(1) 查找。
		//
		// 背景：旧实现对每条 link 都遍历所有 node，再遍历 inputs/outputs 找 pin，导致 O(links * nodes * pins) 的二次复杂度。
		// 改进：Compile 过程中本来就要遍历一次所有 pins 来生成 runtime slots，因此顺便把 pin 信息缓存到哈希表即可，整体复杂度降为 O(nodes + pins + links)。
		// ------------------------------
		struct PinInfo
		{
			SLOT_ID slotId = 0;
			bool isOutput = false;  // true: outputs pin，false: inputs pin
		};
		std::unordered_map<EditorPinID, PinInfo> pinLookup;
		// 预留空间，减少 rehash；links 不一定覆盖所有 pin，但通常同量级
		{
			size_t totalPins = 0;
			for (const auto& n : editor.nodes) totalPins += n.inputs.size() + n.outputs.size();
			pinLookup.reserve(totalPins);
		}

		// 1. 节点转换
		for (const auto& enode : editor.nodes)
		{
			NODE_ID nodeId = static_cast<NODE_ID>(enode.id.Get());
			RuntimeNode rnode;
			rnode.id = nodeId;
			rnode.type = enode.type;
			rnode.execute = registry.Get(enode.type);
			// inputs/outputsBegin 由后续填充
			g.nodes.push_back(rnode);
			nodeIdMap[enode.id] = nodeId;
		}

		// 2. Pin 转换
		for (size_t nidx = 0; nidx < editor.nodes.size(); ++nidx)
		{
			const auto& enode = editor.nodes[nidx];
			NODE_ID nodeId = nodeIdMap[enode.id];
			const uint32_t inputsBegin = static_cast<uint32_t>(g.slots.size());
			for (size_t i = 0; i < enode.inputs.size(); ++i)
			{
				const auto& epin = enode.inputs[i];
				RuntimeSlot slot;
				slot.name = epin.name;
				slot.type = epin.type;
				slot.active = false;
				const SLOT_ID slotId = PushSlot(g, std::move(slot));
				g.slotToNode[slotId] = nodeId;

				// 建立 pin -> slotId 映射，供 link 转换阶段 O(1) 查找
				pinLookup[epin.id] = PinInfo{ slotId, false };
			}
			const uint32_t outputsBegin = static_cast<uint32_t>(g.slots.size());
			for (size_t i = 0; i < enode.outputs.size(); ++i)
			{
				const auto& epin = enode.outputs[i];
				RuntimeSlot slot;
				slot.name = epin.name;
				slot.type = epin.type;
				slot.active = false;
				// 将 editor 节点属性写入 runtime 槽值（示例：Dialogue 的 Text 输出）
				if (enode.type == NodeType::Dialogue && epin.type == SlotType::String && epin.name == "Text")
				{
					auto pit = enode.properties.find("text");
					if (pit != enode.properties.end())
					{
						slot.value = Value(pit->second);
					}
				}
				const SLOT_ID slotId = PushSlot(g, std::move(slot));
				g.slotToNode[slotId] = nodeId;

				// 建立 pin -> slotId 映射，供 link 转换阶段 O(1) 查找
				pinLookup[epin.id] = PinInfo{ slotId, true };
			}
			g.nodes[nidx].inputsBegin = inputsBegin;
			g.nodes[nidx].inputsCount = static_cast<uint32_t>(enode.inputs.size());
			g.nodes[nidx].outputsBegin = outputsBegin;
			g.nodes[nidx].outputsCount = static_cast<uint32_t>(enode.outputs.size());
		}

		// 3. Link 转换
		for (const auto& elink : editor.links)
		{
			// 性能要求：这里不允许再遍历 editor.nodes 查 pin；必须 O(1) 通过 pinLookup 查找
			auto startIt = pinLookup.find(elink.startPinId);
			auto endIt = pinLookup.find(elink.endPinId);
			if (startIt == pinLookup.end() || endIt == pinLookup.end())
			{
				// pin 未找到：可能是脏数据或节点已被删除，直接跳过该 link
				continue;
			}

			const PinInfo& start = startIt->second;
			const PinInfo& end = endIt->second;
			// 约定：link.startPinId 应来自输出 pin，link.endPinId 应来自输入 pin
			// 若不满足，仍然构边也能工作，但这里保守处理：不符合则跳过，避免把数据 pin 当 exec pin 误连
			if (!start.isOutput || end.isOutput) continue;

			SLOT_ID from = start.slotId;
			SLOT_ID to = end.slotId;
			g.edges.push_back(RuntimeEdge(from, to));
		}

		// 4. 构建索引表
		BuildEdgeMaps(g);
		BuildNodeIndexMap(g);

		// 5. 设置入口节点（第一个 Entry 类型节点）
		for (const auto& n : g.nodes)
			if (n.type == NodeType::Entry) { g.entryNodeId = n.id; break; }

		return g;
	}
}
