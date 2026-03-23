/*
* EditorGraphContextMenuImpl：节点/连线右键上下文菜单实现拆分
*/

#include "EditorGraphContextMenuImpl.h"

#include "NodeEditorRegistry.h"

namespace Horizon::NodeGraphEditor
{
	static const char* NodeTypeToString(Horizon::NodeGraphRuntime::NodeType type)
	{
		using NodeType = Horizon::NodeGraphRuntime::NodeType;
		switch (type)
		{
		case NodeType::Invalid: return "Invalid";
		case NodeType::Entry: return "Entry";
		case NodeType::Dialogue: return "Dialogue";
		case NodeType::Delay: return "Delay";
		case NodeType::Branch: return "Branch";
		case NodeType::SetVariable: return "SetVariable";
		case NodeType::GetVariable: return "GetVariable";
		case NodeType::Condition: return "Condition";
		case NodeType::Custom0: return "Custom0";
		case NodeType::Custom1: return "Custom1";
		default: return "Unknown";
		}
	}

	void HandleNodeLinkContextMenusImpl(
		EditorGraph& graph,
		bool& pendingDeleteNode,
		EditorNodeID& pendingDeleteNodeId,
		bool& pendingDeleteLink,
		EditorLinkID& pendingDeleteLinkId
	)
	{
		static EditorNodeID contextNodeId{};
		static EditorLinkID contextLinkId{};

		bool openNodeMenu = false;
		bool openLinkMenu = false;

		// 必须在 Suspend/Resume 之间调用 ShowNodeContextMenu/ShowLinkContextMenu，
		// 以匹配 ImNodeEditor 内部交互状态机。
		ax::NodeEditor::Suspend();
		if (ax::NodeEditor::ShowNodeContextMenu(&contextNodeId))
		{
			openNodeMenu = true;
		}
		else if (ax::NodeEditor::ShowLinkContextMenu(&contextLinkId))
		{
			openLinkMenu = true;
		}

		if (openNodeMenu)
			ImGui::OpenPopup("Node Context Menu");
		else if (openLinkMenu)
			ImGui::OpenPopup("Link Context Menu");

		// ----------------------------
		// Node 上下文菜单
		// ----------------------------

		if (ImGui::BeginPopup("Node Context Menu"))
		{
			EditorNode* node = graph.FindNode(contextNodeId);

			ImGui::TextUnformatted("Node Context Menu");
			ImGui::Separator();

			if (node)
			{
				ImGui::Text("ID: %d", node->id.Get());
				ImGui::Text("Type: %s (%u)",
					NodeTypeToString(node->type),
					static_cast<unsigned>(node->type));
				ImGui::Text("Inputs: %d", static_cast<int>(node->inputs.size()));
				ImGui::Text("Outputs: %d", static_cast<int>(node->outputs.size()));
			}
			else
			{
				ImGui::Text("Unknown node: %d", contextNodeId.Get());
			}

			ImGui::Separator();

			if (ImGui::MenuItem("Delete"))
			{
				pendingDeleteNode = true;
				pendingDeleteNodeId = contextNodeId;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		// ----------------------------
		// Link 上下文菜单
		// ----------------------------
		if (ImGui::BeginPopup("Link Context Menu"))
		{
			ImGui::TextUnformatted("Link Context Menu");
			ImGui::Separator();

			// 先在 graph.links 中找回该 link（用 id 精确匹配）
			EditorLink* link = nullptr;
			for (auto& l : graph.links)
			{
				if (l.id == contextLinkId)
				{
					link = &l;
					break;
				}
			}

			if (link)
			{
				ImGui::Text("Link ID: %d", link->id.Get());

				// From/To：根据 startPin/endPin 的所属 node 推导
				EditorNode* fromNode = nullptr;
				EditorNode* toNode = nullptr;

				{
					auto it = graph.pinOwnerById.find(link->startPinId);
					if (it != graph.pinOwnerById.end())
						fromNode = graph.FindNode(it->second);
				}
				{
					auto it = graph.pinOwnerById.find(link->endPinId);
					if (it != graph.pinOwnerById.end())
						toNode = graph.FindNode(it->second);
				}

				if (fromNode)
					ImGui::Text("From: Node %d (%s)", fromNode->id.Get(), fromNode->name.c_str());
				else
					ImGui::TextUnformatted("From: <unknown>");

				if (toNode)
					ImGui::Text("To: Node %d (%s)", toNode->id.Get(), toNode->name.c_str());
				else
					ImGui::TextUnformatted("To: <unknown>");
			}
			else
			{
				ImGui::Text("Unknown link: %d", contextLinkId.Get());
			}

			ImGui::Separator();

			if (ImGui::MenuItem("Delete"))
			{
				pendingDeleteLink = true;
				pendingDeleteLinkId = contextLinkId;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		ax::NodeEditor::Resume();
	}
}

