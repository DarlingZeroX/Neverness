/*
* Galgame 节点自定义 UI 绘制扩展点
*
* 目的：
* - 将复杂节点 UI（如 DialogueList）从 HNGEditorCore 中彻底移出
* - 由业务模块（VGEditorGalgame）注册 NodeEditorMeta::customDraw 回调实现
*/

#pragma once

#include <HNGEditorCore/Interface/EditorGraph.h>
#include <HNGRuntimeCore/Include/RuntimeContext.h>

namespace VisionGal::Editor
{
	// DialogueList：节点内预览 + 独立编辑面板 + 序列化回 properties["dialogueListJson"]
	void DrawDialogueListCustomUI(
		Horizon::NodeGraphEditor::EditorGraph& graph,
		Horizon::NodeGraphEditor::EditorNode& node,
		const Horizon::NodeGraphRuntime::RuntimeContext* runtimeCtx
	);
}

