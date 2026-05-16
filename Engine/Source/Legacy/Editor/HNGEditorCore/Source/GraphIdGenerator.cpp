#include "GraphIdGenerator.h"

namespace Horizon::NodeGraphEditor
{
	ax::NodeEditor::NodeId GraphIdGenerator::NewNodeId()
	{
		return ax::NodeEditor::NodeId(state.nextNodeId++);
	}

	ax::NodeEditor::PinId GraphIdGenerator::NewPinId()
	{
		return ax::NodeEditor::PinId(state.nextPinId++);
	}

	ax::NodeEditor::LinkId GraphIdGenerator::NewLinkId()
	{
		return ax::NodeEditor::LinkId(state.nextLinkId++);
	}

	void GraphIdGenerator::Reset(const GraphIdState& newState)
	{
		state = newState;
	}

	void GraphIdGenerator::DebugPrintIdState() const
	{
		std::printf(
			"[GraphIdGenerator] nextNodeId=%d nextPinId=%d nextLinkId=%d\n",
			state.nextNodeId,
			state.nextPinId,
			state.nextLinkId
		);
	}
}

