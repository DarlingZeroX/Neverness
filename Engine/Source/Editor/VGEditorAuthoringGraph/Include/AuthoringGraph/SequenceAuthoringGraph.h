/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace VisionGal::Editor
{
	/// Authoring 图节点：与线性 Sequence 条目通过 entryIndex 关联（非执行 VM）。
	struct SequenceAuthoringNode
	{
		uint64_t nodeId = 0;
		unsigned entryIndex = 0;
		float posX = 0.f;
		float posY = 0.f;
	};

	struct SequenceAuthoringEdge
	{
		uint64_t edgeId = 0;
		uint64_t fromNodeId = 0;
		uint64_t toNodeId = 0;
	};

	struct SequenceAuthoringComment
	{
		uint64_t commentId = 0;
		float posX = 0.f;
		float posY = 0.f;
		std::string text;
	};

	/// 仅承载布局、注释、可视化边；不替代 `SequenceDocument` 线性语义。
	class SequenceAuthoringGraph
	{
	public:
		void Clear();
		void EnsureNodeForEntry(unsigned entryIndex, float defaultX, float defaultY);
		void SetNodePosition(uint64_t nodeId, float x, float y);
		void SetNodePositionForEntry(unsigned entryIndex, float x, float y);
		[[nodiscard]] bool TryGetNodeForEntry(unsigned entryIndex, SequenceAuthoringNode& out) const;
		[[nodiscard]] const std::vector<SequenceAuthoringNode>& GetNodes() const { return m_nodes; }
		[[nodiscard]] const std::vector<SequenceAuthoringEdge>& GetEdges() const { return m_edges; }
		[[nodiscard]] const std::vector<SequenceAuthoringComment>& GetComments() const { return m_comments; }

		void AddEdge(uint64_t fromNodeId, uint64_t toNodeId);
		void AddComment(float x, float y, std::string text);

	private:
		[[nodiscard]] uint64_t NextId();

		uint64_t m_nextId = 1;
		std::vector<SequenceAuthoringNode> m_nodes;
		std::vector<SequenceAuthoringEdge> m_edges;
		std::vector<SequenceAuthoringComment> m_comments;
		std::unordered_map<unsigned, size_t> m_entryToNodeIndex;
	};
}
