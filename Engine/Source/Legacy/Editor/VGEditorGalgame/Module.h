/*
* Galgame 专用 NodeGraph 编辑器（VGEditorGalgame 模块）
*
* 说明：
* - 本模块复用 HNGEditorCore 的通用图编辑能力（EditorGraph/CommandSystem/DrawEditorGraph/GraphCompiler/ExecuteGraph 等）
* - 不修改 HNGEditorCore 代码，不复制 HNodeGraphEditor
*
* 兼容性说明：
* - 当前运行时的 NodeType 枚举只包含少量通用类型（Entry/Dialogue/Branch/SetVariable/Delay/Condition/Custom0/Custom1）。
* - 因此本模块在“Galgame 业务节点”与“NodeType”之间做了映射（通过注册 NodeMeta/NodeEditorMeta 实现）。
*/

#pragma once

#include "VGEGExport.h"

#include <memory>
#include <string>

#include <E:/VisionGal/Engine/Source/Editor/HNGEditorCore/Interface/EditorGraph.h>
#include <E:/VisionGal/Engine/Source/Editor/HNGEditorCore/Interface/NodeGraphEditor.h>
#include <HNGRuntimeCore/Include/NodeRegistry.h>

namespace VisionGal::Editor
{
	class DialogueListEditorPanel;

	class VG_GALGAME_EDITOR_API VisualGalgame
	{
	public:
		VisualGalgame();
		~VisualGalgame();

		// 初始化：绑定图、注册节点、生成初始图
		void Initialize();

		// 绘制：Toolbar -> NodeGraph -> Preview
		void DrawEditorWindow();

		// 每帧更新：根据 Play/Pause 状态编译并执行运行时图
		void Update();
		void Update(float deltaTime);

	private:
		// ----------------------------
		// 核心：图编辑与运行时
		// ----------------------------
		Horizon::NodeGraphEditor::EditorGraph m_Graph;

		Horizon::NodeGraphRuntime::NodeRegistry m_Registry;
		Horizon::NodeGraphEditor::NodeEditorRegistry m_NodeEditorRegistry;
		Horizon::NodeGraphEditor::CommandManager m_CommandManager;

		// 统一的通用编辑器内核：所有 Copy/Paste/Undo/Redo/交互由它负责
		Horizon::NodeGraph::HNodeGraphEditor m_CoreEditor;

		// 复杂节点面板：对白编辑器（独立窗口）
		std::unique_ptr<DialogueListEditorPanel> m_DialogueEditor;

	private:
		// 初始化
		void RegisterGalgameNodes();
		void SetupInitialGraph();

		// UI
		void DrawToolbar();
		void DrawPreviewWindow();

		// Dialogue 扩展（预留：独立窗口）
		void OpenDialogueEditor(ax::NodeEditor::NodeId nodeId);
	};
}
