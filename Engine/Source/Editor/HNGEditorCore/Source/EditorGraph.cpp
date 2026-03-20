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
#include "EditorGraph.h"
#include "NodeFactory.h"
#include "CommandSystem.h"
#include <HNGRuntimeCore/Include/Core/RuntimeContext.h>
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <map>
#include <sstream>
#include <unordered_set>

#include "EditorGraphDrawingImpl.h"
#include "EditorGraphActionsImpl.h"
#include "EditorGraphContextMenuImpl.h"

#include "Utilities/builders.h"
#include <HNGRuntimeCore/Include/Core/ExpressionEvaluator.h>

namespace Horizon::NodeGraphEditor
{
	// 字符串匹配 / 节点绘制 / 连线创建删除实现已拆分到 EditorGraphDrawingImpl /
	// EditorGraphActionsImpl，EditorGraph.cpp 内保留 wrapper 入口即可。

	// ------------------------------------------------------------
	// DrawNodeCreateMenu：右键弹出“创建节点”菜单
	// ------------------------------------------------------------
	void DrawNodeCreateMenu(EditorGraph& graph, const ImVec2& spawnPos)
	{
		// implementation moved to EditorGraphDrawingImpl.cpp
		DrawNodeCreateMenuImpl(graph, spawnPos);
	}

	// DrawNodeProperties / ValueToString 等由 EditorGraphDrawingImpl.cpp 负责

	// ------------------------------------------------------------
	// DrawSingleNode：使用 BlueprintNodeBuilder 结构化绘制节点 UI
	// ------------------------------------------------------------

	void DrawSingleNode(
		EditorGraph& graph,
		EditorNode& node,
		const Horizon::NodeGraphRuntime::RuntimeContext* runtimeCtx
	)
	{
		// implementation moved to EditorGraphDrawingImpl.cpp
		DrawSingleNodeImpl(graph, node, runtimeCtx);
	}

	EditorNode* EditorGraph::FindNode(ax::NodeEditor::NodeId id)
	{
		// 优先使用辅助索引（O(1)）
		auto it = nodeIndexById.find(id);
		if (it != nodeIndexById.end())
		{
			size_t idx = it->second;
			if (idx < nodes.size() && nodes[idx].id == id)
				return &nodes[idx];
		}

		// 索引缺失或失配：回退线性扫描，并在找到后修正索引
		for (size_t i = 0; i < nodes.size(); ++i)
		{
			if (nodes[i].id == id)
			{
				nodeIndexById[id] = i;
				return &nodes[i];
			}
		}
		return nullptr;
	}

	EditorPin* EditorGraph::FindPin(ax::NodeEditor::PinId id)
	{
		// 1) 通过 pinOwnerById 快速找到所属节点
		auto ownerIt = pinOwnerById.find(id);
		if (ownerIt != pinOwnerById.end())
		{
			EditorNode* owner = FindNode(ownerIt->second);
			if (owner)
			{
				for (auto& p : owner->inputs)
					if (p.id == id) return &p;
				for (auto& p : owner->outputs)
					if (p.id == id) return &p;
			}
		}

		// 2) 索引缺失或失配：回退线性扫描并修正 pinOwnerById
		for (auto& n : nodes)
		{
			for (auto& p : n.inputs)
				if (p.id == id)
				{
					pinOwnerById[id] = n.id;
					return &p;
				}
			for (auto& p : n.outputs)
				if (p.id == id)
				{
					pinOwnerById[id] = n.id;
					return &p;
				}
		}
		return nullptr;
	}

	void EditorGraph::RebuildIndices()
	{
		nodeIndexById.clear();
		pinOwnerById.clear();

		for (size_t i = 0; i < nodes.size(); ++i)
		{
			EditorNode& n = nodes[i];
			nodeIndexById[n.id] = i;

			for (const auto& p : n.inputs)
				pinOwnerById[p.id] = n.id;
			for (const auto& p : n.outputs)
				pinOwnerById[p.id] = n.id;
		}
	}

	EditorNode& EditorGraph::AddNode(Runtime::NodeType type)
	{
		// 根据 NodeEditorMeta 初始化 EditorNode.properties 的默认值
		auto initFromEditorMeta = [&](EditorNode& node)
		{
			if (!editorRegistry) return;
			const NodeEditorMeta* meta = editorRegistry->Get(type);
			if (!meta) return;
			for (const auto& prop : meta->properties)
			{
				node.properties[prop.name] = prop.defaultValue;
			}
		};

		// 1) registry 缺失：创建占位节点，避免空指针崩溃
		if (!registry)
		{
			EditorNode node;
			node.id = GenNodeId();
			node.type = type;
			node.name = "Missing Registry";
			initFromEditorMeta(node);
			nodes.push_back(std::move(node));
			dirty = true;
			return nodes.back();
		}

		// 2) meta 缺失：创建占位节点，避免崩溃，并提示 type 未注册
		const Runtime::NodeMeta* meta = registry->GetMeta(type);
		if (!meta)
		{
			EditorNode node;
			node.id = GenNodeId();
			node.type = type;
			node.name = "Unregistered NodeType";
			initFromEditorMeta(node);
			nodes.push_back(std::move(node));
			dirty = true;
			return nodes.back();
		}

		// 3) 正常路径：由 NodeMeta 数据驱动创建 pins
		EditorNode node = CreateNodeFromMeta(*meta);
		initFromEditorMeta(node);
		nodes.push_back(std::move(node));
		// 新增节点后更新索引（仅追加一项，避免全量重建）
		EditorNode& back = nodes.back();
		const size_t idx = nodes.size() - 1;
		nodeIndexById[back.id] = idx;
		for (const auto& p : back.inputs)
			pinOwnerById[p.id] = back.id;
		for (const auto& p : back.outputs)
			pinOwnerById[p.id] = back.id;

		dirty = true;
		return nodes.back();
	}

	void HandleCreateLink(EditorGraph& graph)
	{
		HandleCreateLinkImpl(graph);
	}

	void HandleDelete(EditorGraph& graph)
	{
		HandleDeleteImpl(graph);
	}

	void DrawEditorGraph(EditorGraph& graph, const Horizon::NodeGraphRuntime::RuntimeContext* runtimeCtx)
	{
		using namespace ax::NodeEditor;
		//SetCurrentEditor(graph.context);
		graph.context->SetContext();
		Begin("Node Graph");

		// ----------------------------
		// 删除请求缓存（pending）
		// ----------------------------
		// 关键原因：我们在同一帧内会遍历：
		// - `for (auto& node : graph.nodes)` 调用 `DrawSingleNode`
		// - `for (auto& link : graph.links)` 调用 `Link(...)`
		//
		// 如果在上下文菜单的按钮回调中“立刻 Execute 删除命令”，命令会修改
		// `graph.nodes / graph.links`（例如删除节点会 erase vector 元素），这会使
		// 当前 range-for 的迭代器/引用失效，从而引发各种 debug assertion 或崩溃。
		//
		// 因此这里采用两阶段策略：
		// 1) 菜单按钮只写入 pendingDeleteNode / pendingDeleteLink
		// 2) 完成 nodes/links 的遍历后，再统一 Execute 对应 Command
		bool pendingDeleteNode = false;
		EditorNodeID pendingDeleteNodeId{};
		bool pendingDeleteLink = false;
		EditorLinkID pendingDeleteLinkId{};

		// 节点创建菜单：右键弹出（基于 editorRegistry 的 NodeEditorMeta）
		// spawnPos 需要在“节点画布坐标系”下
		ax::NodeEditor::Suspend();
		DrawNodeCreateMenu(graph, ax::NodeEditor::ScreenToCanvas(ImGui::GetMousePos()));
		ax::NodeEditor::Resume();

		// ----------------------------
		// 预处理：构建“执行路径上的节点对”集合，用于高亮连线
		// 说明：
		// - 利用 RuntimeContext::executedNodes 记录的执行顺序，
		//   将相邻节点 (n[i], n[i+1]) 视为一次“控制流跳转”
		// - 在绘制连线时，如果连线两端刚好连接这对节点，则认为该连线在执行路径上
		// ----------------------------
		std::unordered_set<std::uint64_t> executedPairs;
		if (runtimeCtx && runtimeCtx->executedNodes.size() >= 2)
		{
			const auto& seq = runtimeCtx->executedNodes;
			for (size_t i = 0; i + 1 < seq.size(); ++i)
			{
				std::uint64_t key =
					(static_cast<std::uint64_t>(seq[i]) << 32) |
					static_cast<std::uint64_t>(seq[i + 1]);
				executedPairs.insert(key);
			}
		}

		// 绘制所有节点
		for (auto& node : graph.nodes)
		{
			// 节点绘制与拖拽记录逻辑已封装在 DrawSingleNode
			DrawSingleNode(graph, node, runtimeCtx);
		}

		// 绘制所有连线
		for (auto& link : graph.links)
		{
			// 估算连线对应的“起点/终点节点”的 Runtime NODE_ID
			Horizon::NodeGraphRuntime::NODE_ID fromId = 0;
			Horizon::NodeGraphRuntime::NODE_ID toId = 0;

			for (auto& n : graph.nodes)
			{
				for (auto& p : n.outputs)
					if (p.id == link.startPinId)
						fromId = static_cast<Horizon::NodeGraphRuntime::NODE_ID>(n.id.Get());
				for (auto& p : n.inputs)
					if (p.id == link.endPinId)
						toId = static_cast<Horizon::NodeGraphRuntime::NODE_ID>(n.id.Get());
			}

			bool onPath = false;
			if (fromId != 0 && toId != 0 && !executedPairs.empty())
			{
				std::uint64_t key =
					(static_cast<std::uint64_t>(fromId) << 32) |
					static_cast<std::uint64_t>(toId);
				onPath = executedPairs.find(key) != executedPairs.end();
			}

			// 在执行路径上的连线：使用不同颜色高亮
			if (onPath)
			{
				//PushStyleColor(StyleColor_HovLinkBorder, ImColor(255, 220, 120, 255));
				Link(link.id, link.startPinId, link.endPinId);
				//PopStyleColor();
			}
			else
			{
				Link(link.id, link.startPinId, link.endPinId);
			}

			// 说明：Link 上下文菜单不要在此处创建（在遍历 graph.links 的过程中弹窗/执行交互）
			// 否则容易出现重复菜单或触发 ImNodeEditor 内部状态异常。
			// 我们改为在所有 node/link 绘制完成后统一集中处理。
		}

		// ----------------------------
		// 集中处理 Node/Link 上下文菜单（右键）
		// ----------------------------
		HandleNodeLinkContextMenusImpl(
			graph,
			pendingDeleteNode,
			pendingDeleteNodeId,
			pendingDeleteLink,
			pendingDeleteLinkId
		);

		// ----------------------------
		// 执行上下文菜单触发的删除（统一在遍历结束后执行）
		// ----------------------------
		// 删除命令必须走 CommandSystem 才能支持 Undo/Redo。
		// - 如果上层提供了 `graph.commandManager`：通过 ExecuteCommand 入栈
		// - 否则：回退为直接 Execute（保持功能可用）
		if (pendingDeleteNode)
		{
			if (graph.commandManager)
				graph.commandManager->ExecuteCommand(
					std::make_unique<DeleteNodeCommand>(graph, pendingDeleteNodeId)
				);
			else
				DeleteNodeCommand(graph, pendingDeleteNodeId).Execute();
		}

		if (pendingDeleteLink)
		{
			if (graph.commandManager)
				graph.commandManager->ExecuteCommand(
					std::make_unique<DeleteLinkCommand>(graph, pendingDeleteLinkId)
				);
			else
				DeleteLinkCommand(graph, pendingDeleteLinkId).Execute();
		}

		// 处理删除（Delete/Backspace 或拖断连接）
		HandleDelete(graph);

		// 处理新建连线
		HandleCreateLink(graph);

		End();
	}
}