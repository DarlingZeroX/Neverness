/*
* EditorGraphActionsImpl：连线创建/删除逻辑拆分实现
*/

#include "EditorGraphActionsImpl.h"

#include <unordered_set>

#include "CommandSystem.h"

#include <HNGRuntimeCore/Include/Core/RuntimeContext.h>

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
					EditorPin* outputPin = startPin->isInput ? endPin : startPin;
					EditorPin* inputPin = startPin->isInput ? startPin : endPin;
					EditorLink link;
					link.id = ax::NodeEditor::LinkId(static_cast<int>(graph.links.size() + 1));
					link.startPinId = outputPin->id;
					link.endPinId = inputPin->id;

					// 若提供了命令管理器，则通过命令系统创建连线，支持 Undo/Redo
					if (graph.commandManager)
					{
						graph.commandManager->ExecuteCommand(
							std::make_unique<LinkCommand>(graph, link)
						);
					}
					else
					{
						// 回退路径：直接修改 graph.links
						graph.links.push_back(link);
						graph.dirty = true;
					}
				}
			}
			EndCreate();
		}
	}

	void HandleDeleteImpl(EditorGraph& graph)
	{
		using namespace ax::NodeEditor;

		// BeginDelete 会在用户按 Delete/Backspace 或断开连接等“删除动作”激活时返回 true
		if (!BeginDelete())
			return;

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

				if (graph.commandManager)
					graph.commandManager->ExecuteCommand(std::make_unique<DeleteNodeCommand>(graph, nodeId));
				else
					DeleteNodeCommand(graph, nodeId).Execute();

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
					if (graph.commandManager)
						graph.commandManager->ExecuteCommand(std::make_unique<DeleteLinkCommand>(graph, linkId));
					else
						DeleteLinkCommand(graph, linkId).Execute();
				}

				AcceptDeletedItem(false);
				continue;
			}

			break;
		}

		// 如果 BeginDelete 返回 true 但没有任何候选对象（理论上不常见），则拒绝该交互
		if (!didProcessAny)
			RejectDeletedItem();

		EndDelete();
	}
}

