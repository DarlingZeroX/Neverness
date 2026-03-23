#pragma once

#include <cstdio>

#include "../Interface/NodeEditorCore.h" // ax::NodeEditor::NodeId / PinId / LinkId

namespace Horizon::NodeGraphEditor
{
	// 仅保存“下一次应该生成的 id”，保证删除/Undo/Redo 不会回收 id
	struct GraphIdState
	{
		int nextNodeId = 1;
		int nextPinId = 1000;
		int nextLinkId = 10000;
	};

	class GraphIdGenerator
	{
	public:
		ax::NodeEditor::NodeId NewNodeId();
		ax::NodeEditor::PinId NewPinId();
		ax::NodeEditor::LinkId NewLinkId();

		void Reset(const GraphIdState& state);
		GraphIdState GetState() const { return state; }

		// Debug 用：帮助验证加载后 next* 是否正确递增
		void DebugPrintIdState() const;

	private:
		GraphIdState state;
	};
}

