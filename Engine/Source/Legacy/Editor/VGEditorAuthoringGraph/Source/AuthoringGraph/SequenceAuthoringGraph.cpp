/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "AuthoringGraph/SequenceAuthoringGraph.h"

namespace VisionGal::Editor
{
	void SequenceAuthoringGraph::Clear()
	{
		m_nodes.clear();
		m_edges.clear();
		m_comments.clear();
		m_entryToNodeIndex.clear();
		m_nextId = 1;
	}

	uint64_t SequenceAuthoringGraph::NextId()
	{
		return m_nextId++;
	}

	void SequenceAuthoringGraph::EnsureNodeForEntry(
		const unsigned entryIndex,
		const float defaultX,
		const float defaultY)
	{
		if (m_entryToNodeIndex.count(entryIndex))
			return;
		SequenceAuthoringNode n;
		n.nodeId = NextId();
		n.entryIndex = entryIndex;
		n.posX = defaultX;
		n.posY = defaultY;
		m_entryToNodeIndex[entryIndex] = m_nodes.size();
		m_nodes.push_back(n);
	}

	void SequenceAuthoringGraph::SetNodePosition(const uint64_t nodeId, const float x, const float y)
	{
		for (SequenceAuthoringNode& n : m_nodes)
		{
			if (n.nodeId == nodeId)
			{
				n.posX = x;
				n.posY = y;
				return;
			}
		}
	}

	void SequenceAuthoringGraph::SetNodePositionForEntry(
		const unsigned entryIndex,
		const float x,
		const float y)
	{
		const auto it = m_entryToNodeIndex.find(entryIndex);
		if (it == m_entryToNodeIndex.end())
			return;
		SequenceAuthoringNode& n = m_nodes[it->second];
		n.posX = x;
		n.posY = y;
	}

	bool SequenceAuthoringGraph::TryGetNodeForEntry(
		const unsigned entryIndex,
		SequenceAuthoringNode& out) const
	{
		const auto it = m_entryToNodeIndex.find(entryIndex);
		if (it == m_entryToNodeIndex.end())
			return false;
		out = m_nodes[it->second];
		return true;
	}

	void SequenceAuthoringGraph::AddEdge(const uint64_t fromNodeId, const uint64_t toNodeId)
	{
		SequenceAuthoringEdge e;
		e.edgeId = NextId();
		e.fromNodeId = fromNodeId;
		e.toNodeId = toNodeId;
		m_edges.push_back(e);
	}

	void SequenceAuthoringGraph::AddComment(const float x, const float y, std::string text)
	{
		SequenceAuthoringComment c;
		c.commentId = NextId();
		c.posX = x;
		c.posY = y;
		c.text = std::move(text);
		m_comments.push_back(std::move(c));
	}
}
