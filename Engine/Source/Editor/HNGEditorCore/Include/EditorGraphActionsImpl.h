#pragma once

#include "../HNGEditorCoreConfig.h"
#include "EditorGraph.h"

namespace Horizon::NodeGraphEditor
{
	// 连线创建/删除逻辑拆分点：
	// - CreateLink：BeginCreate/QueryNewLink/AcceptNewItem
	// - Delete：BeginDelete/QueryDeleted.../AcceptDeletedItem
	//
	// EditorGraph.cpp 内会用 wrapper 调用这里的 *Impl。
	HNG_EDITOR_CORE_API void HandleCreateLinkImpl(EditorGraph& graph);
	HNG_EDITOR_CORE_API void HandleDeleteImpl(EditorGraph& graph);
}

