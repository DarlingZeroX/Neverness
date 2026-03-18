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
#include <HNGRuntimeCore/Include/Core/RuntimeContext.h>
#include <unordered_set>

namespace Horizon::NodeGraphEditor
{
	EditorNode* EditorGraph::FindNode(ax::NodeEditor::NodeId id)
	{
		for (auto& n : nodes)
		{
			if (n.id == id)
				return &n;
		}
		return nullptr;
	}

	EditorPin* EditorGraph::FindPin(ax::NodeEditor::PinId id)
	{
		for (auto& n : nodes)
		{
			for (auto& p : n.inputs)
				if (p.id == id) return &p;
			for (auto& p : n.outputs)
				if (p.id == id) return &p;
		}
		return nullptr;
	}

	EditorNode& EditorGraph::AddNode(Runtime::NodeType type)
	{
		// 1) registry 缺失：创建占位节点，避免空指针崩溃
		if (!registry)
		{
			EditorNode node;
			node.id = GenNodeId();
			node.type = type;
			node.name = "Missing Registry";
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
			nodes.push_back(std::move(node));
			dirty = true;
			return nodes.back();
		}

		// 3) 正常路径：由 NodeMeta 数据驱动创建 pins
		EditorNode node = CreateNodeFromMeta(*meta);
		nodes.push_back(std::move(node));
		dirty = true;
		return nodes.back();
	}

	void HandleCreateLink(EditorGraph& graph)
	{
		using namespace ax::NodeEditor;
		BeginCreate();

		PinId startPinId, endPinId;
		if (QueryNewLink(&startPinId, &endPinId))
		{
			EditorPin* startPin = graph.FindPin(startPinId);
			EditorPin* endPin = graph.FindPin(endPinId);
			if (!startPin || !endPin)
			{
				RejectNewItem();
				EndCreate();
				return;
			}

			// 校验方向
			if (startPin->isInput == endPin->isInput)
			{
				RejectNewItem();
			}
			// 校验类型
			else if (startPin->type != endPin->type)
			{
				RejectNewItem();
			}
			else if (AcceptNewItem())
			{
				EditorPin* outputPin = startPin->isInput ? endPin : startPin;
				EditorPin* inputPin = startPin->isInput ? startPin : endPin;
				EditorLink link;
				link.id = ax::NodeEditor::LinkId(static_cast<int>(graph.links.size() + 1));
				link.startPinId = outputPin->id;
				link.endPinId = inputPin->id;
				graph.links.push_back(link);
				graph.dirty = true;
			}
		}
		EndCreate();
	}

	void DrawEditorGraph(EditorGraph& graph, const Horizon::NodeGraphRuntime::RuntimeContext* runtimeCtx)
	{
		using namespace ax::NodeEditor;
		//SetCurrentEditor(graph.context);
		graph.context->SetContext();
		Begin("Node Graph");

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
			const Horizon::NodeGraphRuntime::NODE_ID runtimeNodeId = static_cast<Horizon::NodeGraphRuntime::NODE_ID>(node.id.Get());
			const bool executed = runtimeCtx ? runtimeCtx->WasNodeExecuted(runtimeNodeId) : false;
			const bool isCurrent = runtimeCtx ? (runtimeCtx->currentNodeId == runtimeNodeId) : false;

			// 1) 当前执行节点：使用更亮的颜色高亮
			if (isCurrent)
			{
				PushStyleColor(StyleColor_NodeBg, ImColor(240, 200, 80, 230));
				PushStyleColor(StyleColor_NodeBorder, ImColor(255, 230, 120, 255));
			}
			// 2) 已执行节点（但当前不在这里）：使用次级高亮
			else if (executed)
			{
				PushStyleColor(StyleColor_NodeBg, ImColor(70, 120, 255, 200));
				PushStyleColor(StyleColor_NodeBorder, ImColor(120, 180, 255, 255));
				//PushStyleColor(StyleColor_No, ImColor(0, 0, 0, 0));
			}

			BeginNode(node.id);
			ImGui::Text("%s", node.name.c_str());

			// 若是 Dialogue 节点，在节点标题下方显示多行文本编辑框
			// 说明：
			// - 文本存放在 EditorNode.properties["text"] 中
			// - GraphCompiler 会在编译时将该文本写入 RuntimeGraph 的 "Text" 输出槽
			// - 运行时 DialogueNodeExecute 使用该文本按行拆分并逐行播放
			if (node.type == Horizon::NodeGraphRuntime::NodeType::Dialogue)
			{
				std::string& text = node.properties["text"];
				// 为 ImGui 提供一个可修改的缓冲区
				// 这里用静态缓冲示例，若未来需要支持任意长度，可改为动态缓冲管理
				static char buffer[2048];
				std::snprintf(buffer, sizeof(buffer), "%s", text.c_str());
				if (ImGui::InputTextMultiline("Text", buffer, sizeof(buffer),
					ImVec2(0, 0), ImGuiInputTextFlags_AllowTabInput))
				{
					text = buffer;
					graph.dirty = true;
				}
			}

			// 输入区
			for (auto& pin : node.inputs)
			{
				BeginPin(pin.id, PinKind::Input);
				ImGui::Text("%s", pin.name.c_str());
				EndPin();
			}

			ImGui::SameLine();

			// 输出区
			for (auto& pin : node.outputs)
			{
				BeginPin(pin.id, PinKind::Output);
				ImGui::Text("%s", pin.name.c_str());
				EndPin();
			}

			EndNode();

			if (isCurrent || executed)
			{
				PopStyleColor(2);
			}
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
		}

		// 处理新建连线
		HandleCreateLink(graph);

		End();
	}
}