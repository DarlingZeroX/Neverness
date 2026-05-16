/*
* EditorGraphDrawingImpl：节点相关 UI 与交互拆分实现
*/

#include "EditorGraphDrawingImpl.h"

#include <algorithm>
#include <cctype>
#include <unordered_map>
#include <map>
#include <sstream>
#include <vector>

#include "Utilities/builders.h"
#include <HNGRuntimeCore/Include/ExpressionEvaluator.h>
#include "CommandInGraph.h"
#include "GraphCommandAPI.h"

namespace Horizon::NodeGraphEditor
{
	// 多选拖拽合并用的临时缓存：
	// - 在鼠标左键释放那一帧里，所有被拖动的选中节点都会进入 DrawSingleNodeImpl
	// - 我们把每个节点的 oldPos/newPos 记录下来
	// - 然后只在“最后一个被绘制的 selected node”上创建 MultiMoveNodesCommand
	static std::unordered_map<ax::NodeEditor::NodeId, MultiMoveNodesCommand::MoveEntry, EditorIdHash> s_MultiMoveEntries;

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
	// DrawNodeProperties：由 NodeEditorMeta / PropertyMeta 驱动的节点属性 UI
	// ------------------------------------------------------------
	static void DrawNodePropertiesImpl(EditorGraph& graph, EditorNode& node)
	{
		if (!graph.editorRegistry) return;

		const NodeEditorMeta* meta = graph.editorRegistry->Get(node.typeId);
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
				ImGui::SetNextItemWidth(200);
				if (ImGui::InputText(label, buf, sizeof(buf)))
				{
					v = NodeGraphRuntime::Value::FromString(buf);
					graph.dirty = true;
				}
				break;
			}
			case PropertyWidgetType::MultilineText:
			{
				// 目前历史代码中 MultilineText 先保留占位（避免引入未完成控件）。
				break;
			}
			case PropertyWidgetType::Checkbox:
			{
				bool b = v.AsBool();
				if (ImGui::Checkbox(label, &b))
				{
					v = NodeGraphRuntime::Value::FromBool(b);
					graph.dirty = true;
				}
				break;
			}
			case PropertyWidgetType::Float:
			{
				float f = static_cast<float>(v.AsFloat());
				if (ImGui::InputFloat(label, &f))
				{
					v = NodeGraphRuntime::Value::FromFloat(f);
					graph.dirty = true;
				}
				break;
			}
			case PropertyWidgetType::Int:
			{
				int i = static_cast<int>(v.AsInt());
				if (ImGui::InputInt(label, &i))
				{
					v = NodeGraphRuntime::Value::FromInt(i);
					graph.dirty = true;
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

				currentIndex = std::clamp(currentIndex, 0, static_cast<int>(prop.options.size() - 1));

				std::vector<const char*> items;
				items.reserve(prop.options.size());
				for (const auto& opt : prop.options) items.push_back(opt.c_str());

				if (ImGui::Combo(label, &currentIndex, items.data(), static_cast<int>(items.size())))
				{
					if (prop.type == NodeGraphRuntime::ValueType::Int)
						v = NodeGraphRuntime::Value::FromInt(currentIndex);
					else
						v = NodeGraphRuntime::Value::FromString(prop.options[static_cast<size_t>(currentIndex)]);

					graph.dirty = true;
				}
				break;
			}
			default:
				break;
			}
		}
	}

	// ------------------------------------------------------------
	// Value -> string（用于调试信息显示）
	// ------------------------------------------------------------
	static std::string ValueToString(const NodeGraphRuntime::Value& v)
	{
		switch (v.type)
		{
		case NodeGraphRuntime::ValueType::Int:
			return std::to_string(static_cast<int>(v.AsInt()));
		case NodeGraphRuntime::ValueType::Float:
		{
			std::ostringstream oss;
			oss << v.AsFloat();
			return oss.str();
		}
		case NodeGraphRuntime::ValueType::Bool:
			return v.AsBool() ? "true" : "false";
		case NodeGraphRuntime::ValueType::String:
			return v.AsString();
		default:
			return {};
		}
	}

	void DrawSingleNodeImpl(
		EditorGraph& graph,
		EditorNode& node,
		const Horizon::NodeGraphRuntime::RuntimeContext* runtimeCtx
	)
	{
		const Horizon::NodeGraphRuntime::NODE_ID runtimeNodeId =
			static_cast<Horizon::NodeGraphRuntime::NODE_ID>(node.id.Get());
		const bool executed = runtimeCtx ? runtimeCtx->WasNodeExecuted(runtimeNodeId) : false;
		const bool isCurrent = runtimeCtx ? (runtimeCtx->currentNodeId == runtimeNodeId) : false;

		// 根据状态计算 Header 颜色（替代旧的 PushStyleColor(NodeBg/NodeBorder)）
		ImVec4 headerLeftColor = ImVec4(0.28f, 0.28f, 0.28f, 0.35f);
		ImVec4 headerRightColor = ImVec4(0.25f, 0.25f, 0.25f, 0.35f);

		// 运行时调试高亮优先级高于节点类型渐变
		if (isCurrent)
		{
			headerLeftColor = ImVec4(240.0f / 255.0f, 200.0f / 255.0f, 80.0f / 255.0f, 0.9f);
			headerRightColor = ImVec4(255.0f / 255.0f, 140.0f / 255.0f, 60.0f / 255.0f, 0.9f);
		}
		else if (executed)
		{
			headerLeftColor = ImVec4(70.0f / 255.0f, 120.0f / 255.0f, 255.0f / 255.0f, 0.75f);
			headerRightColor = ImVec4(140.0f / 255.0f, 210.0f / 255.0f, 255.0f / 255.0f, 0.75f);
		}
		else
		{
			// 默认渐变：不再依赖旧 NodeType enum
		}

		// ----------------------------
		// Selection highlight（关键）
		// ----------------------------
		// selection 状态来自 DrawEditorGraph 中对 ax::NodeEditor 的同步：
		// graph.selectedNodes / graph.selectedLinks
		//
		// 这里根据 selectedNodes 改变 Header 颜色，让“选中状态”视觉明确。
		if (graph.selectedNodes.find(node.id) != graph.selectedNodes.end())
		{
			// 选择高亮：接近 ImNodeEditor 默认 Selected Node Border 的橙色风格
			headerLeftColor = ImVec4(255.0f / 255.0f, 176.0f / 255.0f, 50.0f / 255.0f, 0.95f);
			headerRightColor = ImVec4(255.0f / 255.0f, 176.0f / 255.0f, 50.0f / 255.0f, 0.95f);
		}

		// 记录移动前的位置，用于 MoveNodeCommand
		ImVec2 oldPos = node.position;

		BlueprintNodeBuilder builder;
		builder.Begin(node.id);
		builder.HeaderGradient(headerLeftColor, headerRightColor);
		ImGui::TextUnformatted(node.name.c_str());
		builder.EndHeader();

		// 左列：Inputs
		for (auto& pin : node.inputs)
			builder.Input(pin.id, pin.name.c_str(), pin.type);
		builder.EndInput();

		// 中间内容：属性 UI + DialogueList（可选） + Debug 信息
		builder.Middle([&]
		{
			DrawNodePropertiesImpl(graph, node);

			// 自定义节点 UI：由外部模块通过 NodeEditorMeta::customDraw 注册
			if (graph.editorRegistry)
			{
				const NodeEditorMeta* meta = graph.editorRegistry->Get(node.typeId);
				if (meta && meta->customDraw)
					meta->customDraw(graph, node, runtimeCtx);
			}

			if (!runtimeCtx)
				return;

			// Debug：SetVariable / Condition
			if (node.properties.find("name") != node.properties.end())
			{
				const std::string varName = node.GetProperty("name").AsString();
				if (!varName.empty())
				{
					auto it = runtimeCtx->variables.find(varName);
					if (it != runtimeCtx->variables.end())
					{
						const std::string valueStr = ValueToString(it->second);
						ImGui::Text("Value: %s", valueStr.empty() ? "<empty>" : valueStr.c_str());
					}
					else
					{
						ImGui::TextUnformatted("Value: <unset>");
					}
				}
			}
			else if (node.properties.find("condition") != node.properties.end())
			{
				const std::string condExpr = node.GetProperty("condition").AsString();
				if (!condExpr.empty())
				{
					// EvaluateExpression 在运行时接口上不使用 const，这里用于调试显示：
					auto& nonConstCtx = *const_cast<Horizon::NodeGraphRuntime::RuntimeContext*>(runtimeCtx);
					Horizon::NodeGraphRuntime::Value res =
						Horizon::NodeGraphRuntime::EvaluateExpression(condExpr, nonConstCtx);
					ImGui::Text("lastCondition: %s", res.AsBool() ? "true" : "false");
				}
			}
		});

		// 右列：Outputs
		for (auto& pin : node.outputs)
			builder.Output(pin.id, pin.name.c_str(), pin.type);
		builder.EndOutput();

		builder.End();

		// 同步并检测节点位置变化（拖拽记录）
		const bool mouseReleased = ImGui::IsMouseReleased(ImGuiMouseButton_Left);
		ImVec2 newPos = GetNodePosition(node.id);
		node.position = newPos;

		if (mouseReleased && graph.commandManager && (newPos.x != oldPos.x || newPos.y != oldPos.y))
		{
			// 多节点拖拽合并：
			// - 当一次拖动涉及多个 selectedNodes 时，每个节点都会在这一帧检测到位置变化
			// - 如果仍对每个节点都执行 MoveNodeCommand，将导致 Undo/Redo 变得很“碎”
			// - 我们改为：记录每个节点的 old/new，然后只在最后一个 selected node 上执行一次 MultiMoveNodesCommand
			if (graph.selectedNodes.size() > 1 && graph.selectedNodes.find(node.id) != graph.selectedNodes.end())
			{
				MultiMoveNodesCommand::MoveEntry entry;
				entry.nodeId = node.id;
				entry.oldPos = oldPos;
				entry.newPos = newPos;

				s_MultiMoveEntries[node.id] = entry;

				// 判定当前 node 是否是“在 graph.nodes 绘制顺序中最后出现的 selected node”
				ax::NodeEditor::NodeId lastSelected{};
				for (auto& n : graph.nodes)
				{
					if (graph.selectedNodes.find(n.id) != graph.selectedNodes.end())
						lastSelected = n.id;
				}

				if (lastSelected == node.id)
				{
					std::vector<MultiMoveNodesCommand::MoveEntry> allEntries;
					allEntries.reserve(graph.selectedNodes.size());

					for (const auto& id : graph.selectedNodes)
					{
						auto it = s_MultiMoveEntries.find(id);
						if (it != s_MultiMoveEntries.end())
							allEntries.push_back(it->second);
					}

					// 防御：理论上 allEntries 应该包含全部 selected nodes
					// 若为空则直接跳过（避免创建空命令破坏历史栈）。
					if (!allEntries.empty())
					{
						graph.commandManager->ExecuteCommand(
							std::make_unique<MultiMoveNodesCommand>(graph, std::move(allEntries))
						);
					}

					s_MultiMoveEntries.clear();
				}
			}
			else
			{
				// 单节点移动：维持原有 MoveNodeCommand 行为
				GraphCommandAPI(graph).MoveNode(node.id, oldPos, newPos);
			}
		}
	}

	// ------------------------------------------------------------
	// DrawNodeCreateMenu：右键弹出“创建节点”菜单
	// ------------------------------------------------------------
	void DrawNodeCreateMenuImpl(EditorGraph& graph, const ImVec2& spawnPos)
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

		if (ax::NodeEditor::ShowBackgroundContextMenu())
		{
			s_LastSpawnPos = spawnPos;
			s_Search[0] = '\0';
			ImGui::OpenPopup("CreateNodePopup");
			ImGui::SetNextWindowPos(ImGui::GetMousePos(), ImGuiCond_Always);
		}

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
					const EditorNodeID newNodeId = GraphCommandAPI(graph).AddNode(meta->typeId, s_LastSpawnPos);
					ax::NodeEditor::SetNodePosition(newNodeId, s_LastSpawnPos);

					created = true;
					ImGui::CloseCurrentPopup();
					break;
				}
			}

			if (created) break;

			if (enterPressed && firstMatch)
			{
				const EditorNodeID newNodeId = GraphCommandAPI(graph).AddNode(firstMatch->typeId, s_LastSpawnPos);
				ax::NodeEditor::SetNodePosition(newNodeId, s_LastSpawnPos);

				created = true;
				ImGui::CloseCurrentPopup();
				break;
			}

			if (enterPressed) break;
		}

		ImGui::EndPopup();
	}
}

