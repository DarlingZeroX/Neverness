/*
 * SelectionSystem：节点选择系统
 *
 * 目标：
 * - 支持单选/多选/框选（框选、多选来自 ax::NodeEditor 内部交互）
 * - 对外提供明确的选择 API，供 UI 高亮、以及批量操作（Delete/Copy/Paste 等）使用
 */
#pragma once

#include <unordered_set>

#include "../HNGEditorCoreConfig.h"
#include "../Interface/EditorGraph.h"

namespace Horizon::NodeGraphEditor
{
	// 用于选择状态的最小封装：
	// - 内部修改 EditorGraph.selectedNodes / selectedLinks
	// - ax::NodeEditor 的“真实选择交互”由 EditorGraph.cpp 同步到 selectedNodes
	class HNG_EDITOR_CORE_API SelectionSystem
	{
	public:
		using NodeId = ax::NodeEditor::NodeId;
		using LinkId = ax::NodeEditor::LinkId;

		explicit SelectionSystem(EditorGraph& graph)
			: m_Graph(graph)
		{
		}

		void SelectNode(NodeId id, bool append)
		{
			// SelectionSystem 的职责：只维护 EditorGraph 里的 selection 数据结构。
			// 框选、Ctrl+Click 等“真实选择交互”由 ax::NodeEditor 内部完成，
			// 我们在 EditorGraph.cpp 里把 ax::NodeEditor 的结果同步写入 selectedNodes/selectedLinks。
			if (!append)
				ClearSelection();
			m_Graph.selectedNodes.insert(id);
		}

		void DeselectNode(NodeId id)
		{
			m_Graph.selectedNodes.erase(id);
		}

		void ClearSelection()
		{
			m_Graph.selectedNodes.clear();
			m_Graph.selectedLinks.clear();
		}

		bool IsSelected(NodeId id) const
		{
			return m_Graph.selectedNodes.find(id) != m_Graph.selectedNodes.end();
		}

		const std::unordered_set<NodeId>& GetSelectedNodes() const
		{
			return m_Graph.selectedNodes;
		}

	private:
		EditorGraph& m_Graph;
	};
}

