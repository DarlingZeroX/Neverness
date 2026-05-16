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
#include "CommandInGraph.h"
#include "GraphCommandAPI.h"
#include <HNGRuntimeCore/Include/RuntimeContext.h>
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <map>
#include <sstream>
#include <vector>
#include <unordered_set>

#include "EditorGraphDrawingImpl.h"
#include "EditorGraphActionsImpl.h"
#include "EditorGraphContextMenuImpl.h"
#include "../Include/SelectionSystem.h"

#include "Utilities/builders.h"
#include <HNGRuntimeCore/Include/ExpressionEvaluator.h>

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

	void EditorGraph::FixupIdStateAfterLoad()
	{
		int maxNodeId = 0;
		int maxPinId = 0;
		int maxLinkId = 0;

		// nodes -> nodeId + pinIds
		for (const auto& n : nodes)
		{
			maxNodeId = std::max(maxNodeId, static_cast<int>(n.id.Get()));
			for (const auto& p : n.inputs)
				maxPinId = std::max(maxPinId, static_cast<int>(p.id.Get()));
			for (const auto& p : n.outputs)
				maxPinId = std::max(maxPinId, static_cast<int>(p.id.Get()));
		}

		// links -> linkId
		for (const auto& l : links)
			maxLinkId = std::max(maxLinkId, static_cast<int>(l.id.Get()));

		// 单调递增：next* 只会向上，不会回退，避免 Undo/Redo/多次加载后的 ID 复用风险
		GraphIdState st = idGen.GetState();
		st.nextNodeId = std::max(st.nextNodeId, maxNodeId + 1);
		st.nextPinId = std::max(st.nextPinId, maxPinId + 1);
		st.nextLinkId = std::max(st.nextLinkId, maxLinkId + 1);
		idGen.Reset(st);
	}

	EditorNode& EditorGraph::AddNode(Runtime::NodeTypeId typeId)
	{
		// 根据 NodeEditorMeta 初始化 EditorNode.properties 的默认值
		auto initFromEditorMeta = [&](EditorNode& node)
		{
			if (!editorRegistry) return;
			const NodeEditorMeta* meta = editorRegistry->Get(typeId);
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
			node.id = idGen.NewNodeId();
			node.typeId = typeId;
			node.name = "Missing Registry";
			initFromEditorMeta(node);
			nodes.push_back(std::move(node));
			dirty = true;
			return nodes.back();
		}

		// 2) meta 缺失：创建占位节点，避免崩溃，并提示 type 未注册
		const Runtime::NodeMeta* meta = registry->GetMeta(typeId);
		if (!meta)
		{
			EditorNode node;
			node.id = idGen.NewNodeId();
			node.typeId = typeId;
			node.name = "Unregistered NodeType";
			initFromEditorMeta(node);
			nodes.push_back(std::move(node));
			dirty = true;
			return nodes.back();
		}

		// 3) 正常路径：由 NodeMeta 数据驱动创建 pins
		EditorNode node = CreateNodeFromMeta(*meta, idGen);
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
		// 选择状态同步（关键）
		// ----------------------------
		// ImNodeEditor 内部完成：
		// - 单击选择节点
		// - Ctrl 多选
		// - 框选（drag marquee）
		//
		// 我们只负责把“ImNodeEditor 的当前 selection 状态”
		// 同步到 EditorGraph.selectedNodes/selectedLinks，供：
		// - DrawSingleNode 的 UI highlight 使用
		// - Delete 等批量操作使用
		SelectionSystem selection(graph);
		// 每帧同步 selection：
		// - ax::NodeEditor 内部完成单选/Ctrl 多选/框选
		// - 我们从 ax::NodeEditor 读取结果，并写回 graph.selectedNodes/graph.selectedLinks
		//
		// 关键：如果我们在本帧通过 Paste/Delete 手动维护了 graph.selectedNodes，
		// 则不应立刻被“旧的 ax selection”覆盖掉。
		auto SyncSelectionFromImNodes = [&]()
		{
			selection.ClearSelection();

			const int selectedCount = ax::NodeEditor::GetSelectedObjectCount();
			if (selectedCount <= 0)
				return;

			// Node
			{
				std::vector<ax::NodeEditor::NodeId> nodeIds(static_cast<size_t>(selectedCount));
				const int filled = ax::NodeEditor::GetSelectedNodes(nodeIds.data(), selectedCount);
				for (int i = 0; i < filled; ++i)
				{
					const ax::NodeEditor::NodeId id = nodeIds[static_cast<size_t>(i)];
					// 防御：Undo/Redo 后 ax::NodeEditor selection 可能残留幽灵 id，
					// 这里过滤，避免把不存在的 node 写回 selectedNodes。
					if (graph.FindNode(id))
						selection.SelectNode(id, true);
				}
			}

			// Link
			{
				std::vector<ax::NodeEditor::LinkId> linkIds(static_cast<size_t>(selectedCount));
				const int filled = ax::NodeEditor::GetSelectedLinks(linkIds.data(), selectedCount);
				for (int i = 0; i < filled; ++i)
				{
					graph.selectedLinks.insert(linkIds[static_cast<size_t>(i)]);
				}
			}
		};

		// 首次进入本帧：仅在
		// - ax selection 确实发生变化；或
		// - graph.selectedNodes 为空（初始化/清空后）
		// 时才同步，避免覆盖 Paste 后的手动 selection。
		if (ax::NodeEditor::HasSelectionChanged() || graph.selectedNodes.empty())
			SyncSelectionFromImNodes();

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

			// 如果本帧期间 selection 发生改变（例如用户正在拖框选），
			// 则在后续节点绘制前立刻刷新 selectedNodes，保证 highlight 更及时。
			if (ax::NodeEditor::HasSelectionChanged())
				SyncSelectionFromImNodes();
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

			// ----------------------------
			// Link 渲染（选择/执行路径优先级）
			// ----------------------------
			// 选择高亮优先级最高：
			// - graph.selectedLinks 中的 link：使用 SelLinkBorder 颜色
			// 执行路径高亮是次级：
			// - onPath=true 的 link：保持当前逻辑（如未来你想打开 PushStyleColor 可在此扩展）
			const bool isSelectedLink =
				graph.selectedLinks.find(link.id) != graph.selectedLinks.end();

			if (isSelectedLink)
			{
				PushStyleColor(StyleColor_SelLinkBorder, ImColor(255, 176, 50, 255));
				Link(link.id, link.startPinId, link.endPinId);
				PopStyleColor();
			}
			else if (onPath)
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
			GraphCommandAPI(graph).DeleteNode(pendingDeleteNodeId);
		}

		if (pendingDeleteLink)
		{
			GraphCommandAPI(graph).DeleteLink(pendingDeleteLinkId);
		}

		// 处理删除（Delete/Backspace 或拖断连接）
		HandleDelete(graph);

		// 处理新建连线
		HandleCreateLink(graph);

		End();
	}
}