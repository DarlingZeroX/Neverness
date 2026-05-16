/*
* EditorGraphActionsImpl：连线创建/删除逻辑拆分实现
*/

#include "EditorGraphActionsImpl.h"

#include <algorithm>
#include <vector>
#include <unordered_set>

#include "CommandInGraph.h"
#include "GraphCommandAPI.h"

#include <VGImgui/IncludeImGui.h>
#include <NNNodeGraphCore/Include/RuntimeContext.h>

namespace Horizon::NodeGraphEditor
{
	void HandleCreateLinkImpl(EditorGraph& graph)
	{
		using namespace ax::NodeEditor;
		// 必须在 BeginCreate() 返回 true 时，才允许调用 QueryNewLink / QueryNewNode，
		// 否则新版 ImNodeEditor 会在 CreateItemAction 未激活时触发断言。
		if (BeginCreate())
		{
			PinId startPinId, endPinId;
			if (QueryNewLink(&startPinId, &endPinId))
			{
				EditorPin* startPin = graph.FindPin(startPinId);
				EditorPin* endPin = graph.FindPin(endPinId);
				if (!startPin || !endPin)
				{
					RejectNewItem();
					EndCreate();
					return;
				}

				// 校验方向
				if (startPin->isInput == endPin->isInput)
				{
					RejectNewItem();
				}
				// 校验类型
				else if (startPin->type != endPin->type)
				{
					RejectNewItem();
				}
				else if (AcceptNewItem())
				{
					GraphCommandAPI(graph).CreateLink(startPinId, endPinId);
				}
			}
			EndCreate();
		}
	}

	void HandleDeleteImpl(EditorGraph& graph)
	{
		using namespace ax::NodeEditor;

		// ------------------------------------------------------------
		// 选择集删除（关键）
		// ------------------------------------------------------------
		// 需求：
		// - 使用 selection system 得到 graph.selectedNodes
		// - 按 Delete 键时，对所有选中的节点执行 DeleteNodeCommand
		// - 删除后清空 selection（graph + ImNodeEditor），避免下一帧 selection 残留导致重复删除/重复入栈
		//
		// 注意：
		// - 现有 ImNodeEditor 的 BeginDelete/QueryDeleted... 也会处理 Delete，
		//   为了避免“同一帧重复执行两套删除逻辑”，我们在已处理 selection-delete 后直接 return。
		if (ImGui::IsKeyPressed(ImGuiKey_Delete, false) &&
			(!graph.selectedNodes.empty() || !graph.selectedLinks.empty()))
		{
			GraphCommandAPI api(graph);
			api.BeginBatch(); // 一次 Delete = 一次 Undo/Redo

			// 1) 删除 selected nodes（确定性排序，保证可复现）
			std::vector<ax::NodeEditor::NodeId> nodeIds;
			nodeIds.reserve(graph.selectedNodes.size());
			for (const auto& id : graph.selectedNodes)
				nodeIds.push_back(id);

			std::sort(nodeIds.begin(), nodeIds.end(),
				[](const ax::NodeEditor::NodeId& a, const ax::NodeEditor::NodeId& b)
				{
					return a.Get() < b.Get();
				});

			// 预收集：将要被删除节点的 pins，用于过滤 selected links（避免重复删除入栈）
			std::unordered_set<ax::NodeEditor::PinId, EditorIdHash> pinsBelongToDeletedNodes;
			pinsBelongToDeletedNodes.reserve(nodeIds.size() * 4);
			for (const auto& nodeId : nodeIds)
			{
				auto* node = graph.FindNode(nodeId);
				if (!node) continue;
				for (const auto& p : node->inputs)  pinsBelongToDeletedNodes.insert(p.id);
				for (const auto& p : node->outputs) pinsBelongToDeletedNodes.insert(p.id);
			}

			for (const auto& id : nodeIds)
				api.DeleteNode(id);

			// 2) 删除 selected links（过滤掉已被 DeleteNode 级联删除的 link）
			std::vector<EditorLink> selectedLinkSnaps;
			selectedLinkSnaps.reserve(graph.selectedLinks.size());
			for (const auto& l : graph.links)
			{
				if (graph.selectedLinks.find(l.id) != graph.selectedLinks.end())
					selectedLinkSnaps.push_back(l);
			}

			std::sort(selectedLinkSnaps.begin(), selectedLinkSnaps.end(),
				[](const EditorLink& a, const EditorLink& b)
				{
					return a.id.Get() < b.id.Get();
				});

			for (const auto& l : selectedLinkSnaps)
			{
				const bool connectedToDeletedNode =
					!pinsBelongToDeletedNodes.empty() &&
					(pinsBelongToDeletedNodes.find(l.startPinId) != pinsBelongToDeletedNodes.end() ||
					 pinsBelongToDeletedNodes.find(l.endPinId) != pinsBelongToDeletedNodes.end());

				if (connectedToDeletedNode)
					continue;

				api.DeleteLink(l.id);
			}

			api.EndBatch();

			// 删除后清空 selection（同时清 ImNodeEditor 内部 selection 状态）
			graph.selectedNodes.clear();
			graph.selectedLinks.clear();
			ax::NodeEditor::ClearSelection();
			return;
		}

		// BeginDelete 会在用户按 Delete/Backspace 或断开连接等“删除动作”激活时返回 true
		if (!BeginDelete())
			return;

		// 本次 BeginDelete..EndDelete 视为一次用户操作：合并为一次 Undo/Redo
		GraphCommandAPI api(graph);
		api.BeginBatch();

		// 记录已经“将要/已经”删除的节点 pin。
		// 当 ImNodeEditor 返回 DeleteLink 候选项时，如果该 link 的 start/end pin 属于被删节点，
		// 则该 link 在我们的 DeleteNodeCommand 中会连带删除，这里就跳过创建 DeleteLinkCommand，
		// 避免重复 Undo/Redo。
		std::unordered_set<PinId, EditorIdHash> deletedPins;

		bool didProcessAny = false;

		// 关键：ImNodeEditor 的删除交互是“候选对象逐个处理”的模型。
		// 不能先把 QueryDeletedNode/QueryDeletedLink 都扫完再只调用一次 AcceptDeletedItem，
		// 否则内部 Candidate 列表索引状态可能失配（你看到的 vector subscript out of range 基本就是这种情况）。
		// 正确做法：Query 到一个对象 -> 我们执行命令 -> 立刻 AcceptDeletedItem() 交给内部删除器移除该对象。
		while (true)
		{
			// 1) 当前候选对象如果是 Node：执行 DeleteNodeCommand
			NodeId nodeId{};
			if (QueryDeletedNode(&nodeId))
			{
				didProcessAny = true;

				EditorNode* node = graph.FindNode(nodeId);
				if (node)
				{
					for (const auto& p : node->inputs) deletedPins.insert(p.id);
					for (const auto& p : node->outputs) deletedPins.insert(p.id);
				}

				api.DeleteNode(nodeId);

				// deleteDependencies=false，避免级联删除依赖导致其他 link 显示/消失异常
				AcceptDeletedItem(false);
				continue;
			}

			// 2) 当前候选对象如果是 Link：执行（或跳过）DeleteLinkCommand
			LinkId linkId{};
			PinId startPinId{};
			PinId endPinId{};
			if (QueryDeletedLink(&linkId, &startPinId, &endPinId))
			{
				didProcessAny = true;

				const bool relatedToDeletedNode =
					(!deletedPins.empty()) &&
					(deletedPins.find(startPinId) != deletedPins.end() ||
					 deletedPins.find(endPinId) != deletedPins.end());

				if (!relatedToDeletedNode)
				{
					api.DeleteLink(linkId);
				}

				AcceptDeletedItem(false);
				continue;
			}

			break;
		}

		// 如果 BeginDelete 返回 true 但没有任何候选对象（理论上不常见），则拒绝该交互
		if (!didProcessAny)
			RejectDeletedItem();

		api.EndBatch();
		EndDelete();
	}
}

