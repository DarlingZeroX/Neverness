#pragma once

#include "../HNGEditorCoreConfig.h"
#include "EditorGraph.h"

namespace Horizon::NodeGraphEditor
{
	// 这些函数是 EditorGraph.cpp 的实现拆分点：
	// - 节点创建菜单（基于 NodeEditorRegistry）
	// - 节点属性 UI（基于 NodeEditorMeta / PropertyMeta）
	// - 单节点绘制（包含运行时调试信息）
	//
	// EditorGraph.cpp 内会用同名 wrapper 调用这里的 *Impl。
	//
	// 之所以拆到单独的 .cpp：
	// - 减少 EditorGraph.cpp 体积
	// - 将 ImGui/UI 相关逻辑与“图交互/命令”逻辑分离，降低耦合

	HNG_EDITOR_CORE_API void DrawNodeCreateMenuImpl(EditorGraph& graph, const ImVec2& spawnPos);
	HNG_EDITOR_CORE_API void DrawSingleNodeImpl(
		EditorGraph& graph,
		EditorNode& node,
		const Horizon::NodeGraphRuntime::RuntimeContext* runtimeCtx
	);
}

