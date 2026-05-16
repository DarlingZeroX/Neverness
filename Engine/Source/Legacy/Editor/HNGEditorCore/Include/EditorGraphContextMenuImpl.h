#pragma once

#include "../Interface/EditorGraph.h"

namespace Horizon::NodeGraphEditor
{
	// 负责把 ImNodeEditor 的 Node/Link 上下文菜单渲染出来，并将菜单里的 Delete 操作写入 pending 标记。
	// 具体的删除执行（DeleteNodeCommand / DeleteLinkCommand）仍由 EditorGraph.cpp 在遍历循环结束后统一 Execute。
	HNG_EDITOR_CORE_API void HandleNodeLinkContextMenusImpl(
		EditorGraph& graph,
		bool& pendingDeleteNode,
		EditorNodeID& pendingDeleteNodeId,
		bool& pendingDeleteLink,
		EditorLinkID& pendingDeleteLinkId
	);
}

