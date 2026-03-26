#pragma once

#include "../Interface/EditorGraph.h"

namespace Horizon::NodeGraphEditor
{
	class CommandManager;

	class HNG_EDITOR_CORE_API GraphCommandAPI
	{
	public:
		explicit GraphCommandAPI(EditorGraph& graph);

		// Node
		ax::NodeEditor::NodeId AddNode(Runtime::NodeTypeId typeId, ImVec2 pos);
		void DeleteNode(ax::NodeEditor::NodeId id);
		void MoveNode(ax::NodeEditor::NodeId id, ImVec2 oldPos, ImVec2 newPos);

		// Link
		void CreateLink(ax::NodeEditor::PinId from, ax::NodeEditor::PinId to);
		void DeleteLink(ax::NodeEditor::LinkId id);

		// 预留：未来用于批量命令打包
		void BeginBatch();
		void EndBatch();

	private:
		EditorGraph& m_Graph;
		CommandManager* m_Cmd = nullptr;
		bool m_InBatch = false;
	};
}

