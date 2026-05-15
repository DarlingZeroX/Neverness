/*
* DialogueListEditorPanel
*
* 说明：
* - 该面板属于 VGEditorGalgame（业务层），用于编辑 DialogueList 复杂节点
* - HNGEditorCore 仅提供 NodeEditorMeta::customDraw 扩展点，不包含任何业务 UI
*/

#pragma once

#include <memory>

#include <HNGEditorCore/Interface/EditorGraph.h>
#include <VGGalgameNodeGraph/Include/DialogueListNodeData.h>

namespace VisionGal::Editor
{
	class DialogueListEditorPanel
	{
	public:
		DialogueListEditorPanel() = default;
		// 注意：本类持有 std::unique_ptr<DialogueListNodeCache>（前置声明），
		// 因此析构函数必须在 .cpp 中定义，确保删除时类型完整。
		~DialogueListEditorPanel();

		// 打开面板并切换到指定节点
		void Open(Horizon::NodeGraphEditor::EditorNode* node);

		// 绘制独立 ImGui 窗口（每帧调用一次）
		void Draw(Horizon::NodeGraphEditor::EditorGraph& graph);

		// 关闭面板并清空状态
		void Close();

		bool IsOpen() const { return m_IsOpen; }

	private:
		// ----------------------------
		// 状态管理（由原 static 变量成员化）
		// ----------------------------
		bool m_IsOpen = false;
		ax::NodeEditor::NodeId m_CurrentNodeId{};
		Horizon::NodeGraphEditor::EditorNode* m_CurrentNode = nullptr;

		bool m_PreviewRunning = false;
		size_t m_PreviewIndex = 0;
		size_t m_PreviewTyped = 0;

		// 当前编辑数据（缓存），写回到 node.properties["dialogueListJson"]
		// 注意：这里直接定义为完整类型，避免 unique_ptr 删除不完整类型导致的编译错误。
		struct DialogueListNodeCache
		{
			VisionGal::Runtime::DialogueListNode data;
			bool dirty = false;
		};
		std::unique_ptr<DialogueListNodeCache> m_Cache;

	private:
		void ResetPreview();
		void EnsureCacheLoadedFromNode();
		void SaveIfDirty(Horizon::NodeGraphEditor::EditorGraph& graph);
	};

	// 全局访问：用于 customDraw 回调中触发 Open()
	// 说明：customDraw 回调只有 (graph,node,ctx)，无法直接拿到 NGEditorGalgame 实例，
	// 因此用一个轻量的业务侧指针桥接。
	void SetDialogueListEditorPanelInstance(DialogueListEditorPanel* panel);
	DialogueListEditorPanel* GetDialogueListEditorPanelInstance();
}

