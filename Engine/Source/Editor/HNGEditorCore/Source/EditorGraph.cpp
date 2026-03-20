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
#include <unordered_set>

namespace Horizon::NodeGraphEditor
{
	static const NodeEditorRegistry* s_EditorRegistry = nullptr;
	static EditorGraph* s_CurrentGraph = nullptr;

	// ------------------------------------------------------------
	// 字符串匹配（忽略大小写，子串匹配）
	// ------------------------------------------------------------
	static bool MatchIgnoreCaseSubstring(const std::string& text, const std::string& keyword)
	{
		if (keyword.empty()) return true;

		auto toLower = [](std::string s)
		{
			for (char& c : s)
			{
				c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
			}
			return s;
		};

		const std::string t = toLower(text);
		const std::string k = toLower(keyword);
		return t.find(k) != std::string::npos;
	}

	// ------------------------------------------------------------
	// DrawNodeCreateMenu：右键弹出“创建节点”菜单
	// ------------------------------------------------------------
	void DrawNodeCreateMenu(EditorGraph& graph, const ImVec2& spawnPos)
	{
		if (!graph.editorRegistry) return;

		static char s_Search[128] = {};
		static ImVec2 s_LastSpawnPos = spawnPos;
		static bool s_Initialized = false;
		if (!s_Initialized)
		{
			s_Initialized = true;
			s_LastSpawnPos = spawnPos;
		}

		// 右键背景：打开弹窗
		// 注意：NodeEditor 的右键“上下文菜单触发”应使用 ShowBackgroundContextMenu()
		// IsBackgroundClicked() 对应的是 SelectButtonIndex（默认左键），因此不能用它判断右键。
		if (ax::NodeEditor::ShowBackgroundContextMenu())
		{
			s_LastSpawnPos = spawnPos;
			s_Search[0] = '\0';
			ImGui::OpenPopup("CreateNodePopup");
			ImGui::SetNextWindowPos(ImGui::GetMousePos(), ImGuiCond_Always);
		}

		// 弹窗绘制
		if (!ImGui::BeginPopup("CreateNodePopup", ImGuiWindowFlags_AlwaysAutoResize))
			return;
		// 1) 搜索框
		ImGui::InputText("Search", s_Search, sizeof(s_Search));

		const std::string keyword = s_Search;

		// 2) 构建：category -> metas
		std::map<std::string, std::vector<const NodeEditorMeta*>> grouped;
		const auto allMetas = graph.editorRegistry->GetAll();
		for (const NodeEditorMeta* meta : allMetas)
		{
			if (!meta) continue;

			bool matched =
				MatchIgnoreCaseSubstring(meta->displayName, keyword) ||
				MatchIgnoreCaseSubstring(meta->category, keyword);

			// PropertyMeta.name（可选加分）：同样参与匹配
			if (!matched && !keyword.empty())
			{
				for (const auto& prop : meta->properties)
				{
					if (MatchIgnoreCaseSubstring(prop.name, keyword))
					{
						matched = true;
						break;
					}
				}
			}

			if (!matched) continue;
			grouped[meta->category].push_back(meta);
		}

		// 3) 分类别展示
		size_t totalMatched = 0;
		for (const auto& kv : grouped) totalMatched += kv.second.size();

		// 如果没有匹配，给提示
		if (totalMatched == 0)
		{
			ImGui::TextUnformatted("No match.");
			ImGui::EndPopup();
			return;
		}

		// 可选增强：Enter 创建第一个匹配节点（简单实现）
		const bool enterPressed = ImGui::IsKeyPressed(ImGuiKey_Enter, false);

		const NodeEditorMeta* firstMatch = nullptr;
		bool created = false;
		for (const auto& kv : grouped)
		{
			const std::string& category = kv.first;
			const auto& list = kv.second;
			if (list.empty()) continue;

			ImGui::SeparatorText(category.c_str());
			for (const NodeEditorMeta* meta : list)
			{
				if (!firstMatch) firstMatch = meta;

				const bool selected = ImGui::Selectable(meta->displayName.c_str());
				if (selected)
				{
					EditorNode& newNode = graph.AddNode(meta->type);
					newNode.position = s_LastSpawnPos;
					ax::NodeEditor::SetNodePosition(newNode.id, s_LastSpawnPos);

					created = true;
					ImGui::CloseCurrentPopup();
					break;
				}
			}

			if (created) break;

			if (enterPressed && firstMatch)
			{
				EditorNode& n = graph.AddNode(firstMatch->type);
				n.position = s_LastSpawnPos;
				ax::NodeEditor::SetNodePosition(n.id, s_LastSpawnPos);

				created = true;
				ImGui::CloseCurrentPopup();
				break;
			}

			// 若 CloseCurrentPopup 在上一轮已经触发，这里也直接跳出
			if (enterPressed) break;
		}

		ImGui::EndPopup();
	}

	// ------------------------------------------------------------
	// DrawNodeProperties：由 NodeEditorMeta 驱动的节点属性 UI
	// ------------------------------------------------------------
	void DrawNodeProperties(EditorNode& node)
	{
		if (!s_EditorRegistry) return;

		const NodeEditorMeta* meta = s_EditorRegistry->Get(node.type);
		if (!meta) return;

		for (const auto& prop : meta->properties)
		{
			auto& v = node.GetProperty(prop.name);
			const char* label = prop.displayName.empty() ? prop.name.c_str() : prop.displayName.c_str();

			switch (prop.widget)
			{
			case PropertyWidgetType::InputText:
			{
				char buf[1024];
				std::snprintf(buf, sizeof(buf), "%s", v.AsString().c_str());
				if (ImGui::InputText(label, buf, sizeof(buf)))
				{
					v = NodeGraphRuntime::Value::FromString(buf);
					if (s_CurrentGraph) s_CurrentGraph->dirty = true;
				}
				break;
			}
			case PropertyWidgetType::MultilineText:
			{
				char buf[2048];
				std::snprintf(buf, sizeof(buf), "%s", v.AsString().c_str());
				if (ImGui::InputTextMultiline(label, buf, sizeof(buf),
					ImVec2(0, 0), ImGuiInputTextFlags_AllowTabInput))
				{
					v = NodeGraphRuntime::Value::FromString(buf);
					if (s_CurrentGraph) s_CurrentGraph->dirty = true;
				}
				break;
			}
			case PropertyWidgetType::Checkbox:
			{
				bool b = v.AsBool();
				if (ImGui::Checkbox(label, &b))
				{
					v = NodeGraphRuntime::Value::FromBool(b);
					if (s_CurrentGraph) s_CurrentGraph->dirty = true;
				}
				break;
			}
			case PropertyWidgetType::Float:
			{
				float f = static_cast<float>(v.AsFloat());
				if (ImGui::InputFloat(label, &f))
				{
					v = NodeGraphRuntime::Value::FromFloat(f);
					if (s_CurrentGraph) s_CurrentGraph->dirty = true;
				}
				break;
			}
			case PropertyWidgetType::Int:
			{
				int i = static_cast<int>(v.AsInt());
				if (ImGui::InputInt(label, &i))
				{
					v = NodeGraphRuntime::Value::FromInt(i);
					if (s_CurrentGraph) s_CurrentGraph->dirty = true;
				}
				break;
			}
			case PropertyWidgetType::Combo:
			{
				if (prop.options.empty()) break;

				int currentIndex = 0;
				if (prop.type == NodeGraphRuntime::ValueType::Int)
				{
					currentIndex = static_cast<int>(v.AsInt());
				}
				else
				{
					const std::string cur = v.AsString();
					auto it = std::find(prop.options.begin(), prop.options.end(), cur);
					currentIndex = (it != prop.options.end())
						? static_cast<int>(std::distance(prop.options.begin(), it))
						: 0;
				}

				if (currentIndex < 0) currentIndex = 0;
				if (currentIndex >= static_cast<int>(prop.options.size()))
					currentIndex = static_cast<int>(prop.options.size() - 1);

				std::vector<const char*> items;
				items.reserve(prop.options.size());
				for (const auto& opt : prop.options) items.push_back(opt.c_str());

				if (ImGui::Combo(label, &currentIndex, items.data(), static_cast<int>(items.size())))
				{
					if (prop.type == NodeGraphRuntime::ValueType::Int)
						v = NodeGraphRuntime::Value::FromInt(currentIndex);
					else
						v = NodeGraphRuntime::Value::FromString(prop.options[static_cast<size_t>(currentIndex)]);

					if (s_CurrentGraph) s_CurrentGraph->dirty = true;
				}
				break;
			}
			default:
				break;
			}
		}
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
		using namespace ax::NodeEditor;
		// 必须在 BeginCreate() 返回 true 时，才允许调用 QueryNewLink / QueryNewNode，
		// 否则新版 ImNodeEditor 会在 CreateItemAction 未激活时触发断言。
		if (BeginCreate())
		{
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

					// 若提供了命令管理器，则通过命令系统创建连线，支持 Undo/Redo
					if (graph.commandManager)
					{
						graph.commandManager->ExecuteCommand(
							std::make_unique<LinkCommand>(graph, link)
						);
					}
					else
					{
						// 回退路径：直接修改 graph.links
						graph.links.push_back(link);
						graph.dirty = true;
					}
				}
			}
			EndCreate();
		}
	}

	void DrawEditorGraph(EditorGraph& graph, const Horizon::NodeGraphRuntime::RuntimeContext* runtimeCtx)
	{
		using namespace ax::NodeEditor;
		//SetCurrentEditor(graph.context);
		graph.context->SetContext();
		s_EditorRegistry = graph.editorRegistry;
		s_CurrentGraph = &graph;
		Begin("Node Graph");

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

		const bool mouseReleased = ImGui::IsMouseReleased(ImGuiMouseButton_Left);

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

			// 记录移动前的位置，用于 MoveNodeCommand
			ImVec2 oldPos = node.position;

			BeginNode(node.id);
			ImGui::Text("%s", node.name.c_str());

			// 通用：节点属性 UI（由 NodeEditorMeta 驱动）
			DrawNodeProperties(node);

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

			// 同步并检测节点位置变化
			ImVec2 newPos = GetNodePosition(node.id);
			node.position = newPos;
			if (mouseReleased && graph.commandManager &&
				(newPos.x != oldPos.x || newPos.y != oldPos.y))
			{
				graph.commandManager->ExecuteCommand(
					std::make_unique<MoveNodeCommand>(graph, node.id, oldPos, newPos)
				);
			}

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