/*
* 命令系统实现（Undo/Redo）
*/

#include "CommandSystem.h"

#include <algorithm>

namespace Horizon::NodeGraphEditor
{
	// ---------------- CommandManager ----------------

	void CommandManager::ExecuteCommand(std::unique_ptr<ICommand> cmd)
	{
		if (!cmd) return;
		cmd->Execute();

		// 一旦执行了新命令，redo 历史就失效（与绝大多数编辑器一致）
		m_RedoStack.clear();
		m_UndoStack.push_back(std::move(cmd));
	}

	void CommandManager::Undo()
	{
		if (m_UndoStack.empty()) return;
		auto cmd = std::move(m_UndoStack.back());
		m_UndoStack.pop_back();
		cmd->Undo();
		m_RedoStack.push_back(std::move(cmd));
	}

	void CommandManager::Redo()
	{
		if (m_RedoStack.empty()) return;
		auto cmd = std::move(m_RedoStack.back());
		m_RedoStack.pop_back();
		cmd->Execute();
		m_UndoStack.push_back(std::move(cmd));
	}

	void CommandManager::Clear()
	{
		m_UndoStack.clear();
		m_RedoStack.clear();
	}

	// ---------------- 辅助函数 ----------------

	static bool NodeOwnsPin(const EditorNode& node, const ax::NodeEditor::PinId pinId)
	{
		for (const auto& p : node.inputs)  if (p.id == pinId) return true;
		for (const auto& p : node.outputs) if (p.id == pinId) return true;
		return false;
	}

	static void RemoveLinksOfNode(EditorGraph& graph, const ax::NodeEditor::NodeId nodeId, std::vector<EditorLink>* removed = nullptr)
	{
		EditorNode* n = graph.FindNode(nodeId);
		if (!n) return;

		auto isRelated = [&](const EditorLink& l)
		{
			return NodeOwnsPin(*n, l.startPinId) || NodeOwnsPin(*n, l.endPinId);
		};

		auto it = graph.links.begin();
		while (it != graph.links.end())
		{
			if (isRelated(*it))
			{
				if (removed) removed->push_back(*it);
				it = graph.links.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	static void RemoveNodeById(EditorGraph& graph, const ax::NodeEditor::NodeId nodeId, std::optional<EditorNode>* removed = nullptr)
	{
		auto it = std::find_if(graph.nodes.begin(), graph.nodes.end(),
			[&](const EditorNode& n) { return n.id == nodeId; });
		if (it == graph.nodes.end()) return;
		if (removed) *removed = *it;
		graph.nodes.erase(it);
	}

	static void RemoveLinkById(EditorGraph& graph, const ax::NodeEditor::LinkId linkId)
	{
		auto it = std::find_if(graph.links.begin(), graph.links.end(),
			[&](const EditorLink& l) { return l.id == linkId; });
		if (it != graph.links.end())
			graph.links.erase(it);
	}

	// ---------------- AddNodeCommand ----------------

	AddNodeCommand::AddNodeCommand(EditorGraph& graph, const EditorNode& nodeToAdd)
		: m_Graph(graph)
		, m_Node(nodeToAdd)
	{
	}

	void AddNodeCommand::Execute()
	{
		// 只要执行，就把节点插入图中（若已存在同 id，则不重复插入）
		if (m_Graph.FindNode(m_Node.id))
			return;

		m_Graph.nodes.push_back(m_Node);
		m_Graph.dirty = true;
		m_Executed = true;
	}

	void AddNodeCommand::Undo()
	{
		if (!m_Executed) return;
		RemoveLinksOfNode(m_Graph, m_Node.id);
		RemoveNodeById(m_Graph, m_Node.id);
		m_Graph.dirty = true;
	}

	// ---------------- DeleteNodeCommand ----------------

	DeleteNodeCommand::DeleteNodeCommand(EditorGraph& graph, ax::NodeEditor::NodeId nodeId)
		: m_Graph(graph)
		, m_NodeId(nodeId)
	{
	}

	void DeleteNodeCommand::Execute()
	{
		// 若节点不存在，直接忽略
		if (!m_Graph.FindNode(m_NodeId))
			return;

		// 1) 先移除相关连线并保存快照
		m_DeletedLinks.clear();
		RemoveLinksOfNode(m_Graph, m_NodeId, &m_DeletedLinks);

		// 2) 再移除节点并保存快照
		m_DeletedNode.reset();
		RemoveNodeById(m_Graph, m_NodeId, &m_DeletedNode);

		m_Graph.dirty = true;
		m_Executed = true;
	}

	void DeleteNodeCommand::Undo()
	{
		if (!m_Executed) return;
		if (!m_DeletedNode.has_value()) return;

		// 1) 恢复节点（保持原 id / pins）
		if (!m_Graph.FindNode(m_DeletedNode->id))
			m_Graph.nodes.push_back(*m_DeletedNode);

		// 2) 恢复连线（保持原 id）
		for (const auto& l : m_DeletedLinks)
		{
			auto it = std::find_if(m_Graph.links.begin(), m_Graph.links.end(),
				[&](const EditorLink& x) { return x.id == l.id; });
			if (it == m_Graph.links.end())
				m_Graph.links.push_back(l);
		}

		m_Graph.dirty = true;
	}

	// ---------------- LinkCommand ----------------

	LinkCommand::LinkCommand(EditorGraph& graph, const EditorLink& link)
		: m_Graph(graph)
		, m_Link(link)
	{
	}

	void LinkCommand::Execute()
	{
		// 若 link 已存在则不重复插入
		auto it = std::find_if(m_Graph.links.begin(), m_Graph.links.end(),
			[&](const EditorLink& l) { return l.id == m_Link.id; });
		if (it != m_Graph.links.end())
			return;

		m_Graph.links.push_back(m_Link);
		m_Graph.dirty = true;
		m_Executed = true;
	}

	void LinkCommand::Undo()
	{
		if (!m_Executed) return;
		RemoveLinkById(m_Graph, m_Link.id);
		m_Graph.dirty = true;
	}

	// ---------------- MoveNodeCommand ----------------

	MoveNodeCommand::MoveNodeCommand(EditorGraph& graph, ax::NodeEditor::NodeId nodeId, ImVec2 oldPos, ImVec2 newPos)
		: m_Graph(graph)
		, m_NodeId(nodeId)
		, m_OldPos(oldPos)
		, m_NewPos(newPos)
	{
	}

	void MoveNodeCommand::Execute()
	{
		EditorNode* n = m_Graph.FindNode(m_NodeId);
		if (!n) return;
		n->position = m_NewPos;
		m_Graph.dirty = true;
		m_Executed = true;
	}

	void MoveNodeCommand::Undo()
	{
		if (!m_Executed) return;
		EditorNode* n = m_Graph.FindNode(m_NodeId);
		if (!n) return;
		n->position = m_OldPos;
		m_Graph.dirty = true;
	}
}

