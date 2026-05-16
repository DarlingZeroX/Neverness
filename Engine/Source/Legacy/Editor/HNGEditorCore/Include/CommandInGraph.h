/*
* 命令系统（Command System）
*
* 🎯 目标：
* - 为蓝图编辑器提供可撤销/重做（Undo/Redo）的基础设施
* - 将“对图的修改”封装成命令对象，统一管理执行与回滚
*
* 为什么需要命令系统？
* - 编辑器操作（AddNode/DeleteNode/Link/MoveNode）都需要可撤销
* - 如果直接在 UI 回调里修改数据，很难可靠地恢复到历史状态
* - 命令对象天然包含“做/撤销”两套逻辑，更适合编辑器
*
* 设计原则：
* 1) 命令必须是“可逆的”
*    - Execute() 改变 EditorGraph 状态
*    - Undo() 必须能把状态恢复到 Execute() 之前
*
* 2) 命令要保存足够的快照数据
*    - DeleteNode 需要保存被删节点以及相关连线
*    - MoveNode 需要保存移动前/后的位置
*    - AddNode 需要保存创建出来的节点（含 id/pins），否则重做会生成不同 id 导致 link 断裂
*
* 3) CommandManager 负责：
*    - 执行命令并压入 undo 栈
*    - Undo：弹出 undo 栈并调用 Undo()，压入 redo 栈
*    - Redo：弹出 redo 栈并调用 Execute()，压回 undo 栈
*/

#pragma once

#include <memory>
#include <vector>
#include <optional>
#include <unordered_set>
#include "../HNGEditorCoreConfig.h"
#include "../Interface/EditorGraph.h"
#include "CommandSystem.h"

namespace Horizon::NodeGraphEditor
{
	// ----------------------------
	// Copy / Paste 数据快照
	// ----------------------------
	// CopyBuffer 只是一份“内存快照”，不走 CommandSystem（Copy 本身不改变图状态）。
	// Paste 必须走 Command，确保 Undo/Redo 可恢复“插入的 nodes + links”。
	struct NodeGraphCopyBuffer
	{
		std::vector<EditorNode> nodes;
		std::vector<EditorLink> links;
	};

	// ----------------------------
	// 具体命令
	// ----------------------------

	// AddNodeCommand：新增节点（可撤销）
	// - Execute：把保存的节点插入 graph.nodes
	// - Undo：按 NodeId 删除该节点及其相关连线
	class HNG_EDITOR_CORE_API AddNodeCommand final : public ICommand
	{
	public:
		AddNodeCommand(EditorGraph& graph, const EditorNode& nodeToAdd);
		void Execute() override;
		void Undo() override;

		// 便于外部在创建后拿到 node id（例如立即选中/连线）
		ax::NodeEditor::NodeId GetNodeId() const { return m_Node.id; }

	private:
		EditorGraph& m_Graph;
		EditorNode m_Node;
		bool m_Executed = false;
	};

	// DeleteNodeCommand：删除节点（可撤销）
	// - Execute：删除节点，并移除与其相关的所有 links
	// - Undo：恢复节点与 links（保持原 id / pins 不变）
	// - 索引一致性：删除/恢复后会调用 graph.RebuildIndices()，
	//   确保 nodeIndexById / pinOwnerById 不会残留旧映射
	class HNG_EDITOR_CORE_API DeleteNodeCommand final : public ICommand
	{
	public:
		DeleteNodeCommand(EditorGraph& graph, ax::NodeEditor::NodeId nodeId);
		void Execute() override;
		void Undo() override;

	private:
		EditorGraph& m_Graph;
		ax::NodeEditor::NodeId m_NodeId;

		// 快照：被删除的节点与连线
		std::optional<EditorNode> m_DeletedNode;
		std::vector<EditorLink> m_DeletedLinks;
		bool m_Executed = false;
	};

	// LinkCommand：创建/删除连线（可撤销）
	// - Execute：添加 link
	// - Undo：删除 link
	class HNG_EDITOR_CORE_API LinkCommand final : public ICommand
	{
	public:
		LinkCommand(EditorGraph& graph, const EditorLink& link);
		void Execute() override;
		void Undo() override;

	private:
		EditorGraph& m_Graph;
		EditorLink m_Link;
		bool m_Executed = false;
	};

	// DeleteLinkCommand：删除连线（可撤销）
	// - Execute：保存被删 link 的快照，并从 graph.links 移除
	// - Undo：恢复保存的 link（保持原 id / pin id）
	// - 索引一致性：由于图的 UI 查询依赖 pin->owner 映射，
	//   DeleteLink 结束后也会调用 graph.RebuildIndices()（实现简单且安全）
	class HNG_EDITOR_CORE_API DeleteLinkCommand final : public ICommand
	{
	public:
		DeleteLinkCommand(EditorGraph& graph, ax::NodeEditor::LinkId linkId);
		void Execute() override;
		void Undo() override;

	private:
		EditorGraph& m_Graph;
		ax::NodeEditor::LinkId m_LinkId;

		// 快照：被删除的 EditorLink
		std::optional<EditorLink> m_DeletedLink;
		bool m_Executed = false;
	};

	// MoveNodeCommand：移动节点（可撤销）
	// - Execute：把节点位置设为 newPos
	// - Undo：把节点位置恢复为 oldPos
	class HNG_EDITOR_CORE_API MoveNodeCommand final : public ICommand
	{
	public:
		MoveNodeCommand(EditorGraph& graph, ax::NodeEditor::NodeId nodeId, ImVec2 oldPos, ImVec2 newPos);
		void Execute() override;
		void Undo() override;

	private:
		EditorGraph& m_Graph;
		ax::NodeEditor::NodeId m_NodeId;
		ImVec2 m_OldPos{};
		ImVec2 m_NewPos{};
		bool m_Executed = false;
	};

	// MultiMoveNodesCommand：批量移动多个节点（可撤销）
	// - Execute：把多个 node 的 position 同时更新为 newPos
	// - Undo：把 position 同时恢复为 oldPos
	//
	// 用于“拖拽多个选中节点”时，把本来会生成 N 次 MoveNodeCommand 的行为
	// 合并为一次 Undo/Redo 友好的批量命令。
	class HNG_EDITOR_CORE_API MultiMoveNodesCommand final : public ICommand
	{
	public:
		struct MoveEntry
		{
			ax::NodeEditor::NodeId nodeId{};
			ImVec2 oldPos{};
			ImVec2 newPos{};
		};

		MultiMoveNodesCommand(EditorGraph& graph, std::vector<MoveEntry> entries);
		void Execute() override;
		void Undo() override;

	private:
		EditorGraph& m_Graph;
		std::vector<MoveEntry> m_Entries;
	};

	// PasteNodesCommand：把 CopyBuffer 里的节点/连线复制到图中（可撤销）
	// - Execute：
	//   1) 生成“新 NodeId/PinId/LinkId”映射（只在第一次生成）
	//   2) 根据 offset 平移节点位置
	//   3) 插入 nodes/links，并重建索引
	// - Undo：
	//   1) 删除插入过的 nodes 与其相关 links
	//   2) 重建索引
	class HNG_EDITOR_CORE_API PasteNodesCommand final : public ICommand
	{
	public:
		PasteNodesCommand(EditorGraph& graph, const NodeGraphCopyBuffer& buffer, ImVec2 offset);

		void Execute() override;
		void Undo() override;

		// 供 UI 层（Ctrl+V 后）把选择高亮到新粘贴的节点
		const std::vector<ax::NodeEditor::NodeId>& GetPastedNodeIds() const { return m_PastedNodeIds; }

	private:
		void GenerateIfNeeded();

		EditorGraph& m_Graph;
		const NodeGraphCopyBuffer& m_Buffer;
		ImVec2 m_Offset{};

		bool m_Generated = false;

		// 生成后的“插入物”（new ids 已经就绪）
		std::vector<EditorNode> m_PastedNodes;
		std::vector<EditorLink> m_PastedLinks;
		std::vector<ax::NodeEditor::NodeId> m_PastedNodeIds;

		// Undo 删除用：存 pin 集合（由 pasted nodes 派生）
		std::unordered_set<ax::NodeEditor::PinId> m_PastedPinIds;
	};
}

