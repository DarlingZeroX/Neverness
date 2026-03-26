#include "GraphCommandAPI.h"

#include <algorithm>
#include <cstdio>

#include "CommandInGraph.h"

namespace Horizon::NodeGraphEditor
{
	GraphCommandAPI::GraphCommandAPI(EditorGraph& graph)
		: m_Graph(graph)
		, m_Cmd(graph.commandManager)
	{
	}

	ax::NodeEditor::NodeId GraphCommandAPI::AddNode(Runtime::NodeType type, ImVec2 pos)
	{
		// 统一走 graph.AddNode，保证 NodeMeta 驱动创建与 idGen 行为一致
		EditorNode& created = m_Graph.AddNode(type);
		created.position = pos;
		const EditorNode snapshot = created;

		if (!m_Cmd)
			return snapshot.id;

		// 已创建节点转为命令执行：先移除当前节点，再通过 AddNodeCommand 执行一次
		m_Graph.nodes.erase(
			std::remove_if(
				m_Graph.nodes.begin(),
				m_Graph.nodes.end(),
				[&](const EditorNode& n) { return n.id == snapshot.id; }
			),
			m_Graph.nodes.end()
		);
		m_Graph.RebuildIndices();

		m_Cmd->ExecuteCommand(std::make_unique<AddNodeCommand>(m_Graph, snapshot));
		std::printf("[CMD] AddNode %d\n", snapshot.id.Get());
		return snapshot.id;
	}

	void GraphCommandAPI::DeleteNode(ax::NodeEditor::NodeId id)
	{
		if (m_Cmd)
		{
			m_Cmd->ExecuteCommand(std::make_unique<DeleteNodeCommand>(m_Graph, id));
			std::printf("[CMD] DeleteNode %d\n", id.Get());
			return;
		}

		DeleteNodeCommand(m_Graph, id).Execute();
	}

	void GraphCommandAPI::MoveNode(ax::NodeEditor::NodeId id, ImVec2 oldPos, ImVec2 newPos)
	{
		if (m_Cmd)
		{
			m_Cmd->ExecuteCommand(std::make_unique<MoveNodeCommand>(m_Graph, id, oldPos, newPos));
			std::printf("[CMD] MoveNode %d\n", id.Get());
			return;
		}

		MoveNodeCommand(m_Graph, id, oldPos, newPos).Execute();
	}

	void GraphCommandAPI::CreateLink(ax::NodeEditor::PinId from, ax::NodeEditor::PinId to)
	{
		EditorPin* startPin = m_Graph.FindPin(from);
		EditorPin* endPin = m_Graph.FindPin(to);
		if (!startPin || !endPin)
			return;

		// 与 UI 旧逻辑一致：必须 input/output 方向相反且类型匹配
		if (startPin->isInput == endPin->isInput)
			return;
		if (startPin->type != endPin->type)
			return;

		EditorPin* outputPin = startPin->isInput ? endPin : startPin;
		EditorPin* inputPin = startPin->isInput ? startPin : endPin;

		EditorLink link;
		link.id = m_Graph.idGen.NewLinkId();
		link.startPinId = outputPin->id;
		link.endPinId = inputPin->id;

		if (m_Cmd)
		{
			m_Cmd->ExecuteCommand(std::make_unique<LinkCommand>(m_Graph, link));
			std::printf("[CMD] CreateLink %d\n", link.id.Get());
			return;
		}

		LinkCommand(m_Graph, link).Execute();
	}

	void GraphCommandAPI::DeleteLink(ax::NodeEditor::LinkId id)
	{
		if (m_Cmd)
		{
			m_Cmd->ExecuteCommand(std::make_unique<DeleteLinkCommand>(m_Graph, id));
			std::printf("[CMD] DeleteLink %d\n", id.Get());
			return;
		}

		DeleteLinkCommand(m_Graph, id).Execute();
	}

	void GraphCommandAPI::BeginBatch()
	{
		m_InBatch = true;
		if (m_Cmd) m_Cmd->BeginBatch();
	}

	void GraphCommandAPI::EndBatch()
	{
		if (m_Cmd) m_Cmd->EndBatch();
		m_InBatch = false;
	}
}

