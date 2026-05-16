#include "RuntimeContext.h"

namespace Horizon::NodeGraphRuntime
{
	// ExecuteGraph 实现：基于 execStack 的主循环
	void ExecuteGraph(RuntimeContext& ctx)
	{
		if (!ctx.graph) return;

		// 最大执行步数，防止无限循环（可调整或移除）
		const uint64_t MAX_STEPS = 1000000;
		uint64_t steps = 0;

		while (!ctx.execStack.empty() && steps < MAX_STEPS)
		{
			++steps;

			NODE_ID nid = ctx.execStack.back();
			ctx.execStack.pop_back();

			// 使用 nodeIdToIndex 执行 O(1) 查找
			int idx = GetNodeIndex(*ctx.graph, nid);
			if (idx < 0) continue;

			RuntimeNode& node = ctx.graph->nodes[static_cast<size_t>(idx)];
			if (!node.execute) continue;

			ctx.executedNodes.push_back(node.id);
			ctx.executedNodeSet.insert(node.id);

			// 让 AdvanceToNextNode(ctx) 能获取“当前节点”
			ctx.currentNodeId = node.id;

			// 调用节点执行函数
			ExecResult res = node.execute(ctx, node.id);

			if (res == ExecResult::Running)
			{
				// 仍在运行：
				// - 将节点重新压回执行栈
				// - 立刻中断本轮 ExecuteGraph，避免在单帧内 busy-loop
				ctx.execStack.push_back(node.id);
				break;
			}
			else
			{
				// 已完成：推进到下游节点（可能将多个目标入栈）
				AdvanceToNextNode(ctx);
			}
		}

		// steps 超限时停止执行
	}

	// Push exec token into a slot (mark active)
	void PushExec(RuntimeContext& ctx, SLOT_ID slotId)
	{
		if (!ctx.graph) return;
		if (slotId >= ctx.graph->slots.size()) return;
		ctx.graph->slots[slotId].active = true;
	}

	// Consume exec token from a slot (if active, clear and return true)
	bool ConsumeExec(RuntimeContext& ctx, SLOT_ID slotId)
	{
		if (!ctx.graph) return false;
		if (slotId >= ctx.graph->slots.size()) return false;
		RuntimeSlot& s = ctx.graph->slots[slotId];
		if (s.active)
		{
			s.active = false;
			return true;
		}
		return false;
	}

	// Find next node by following edge from a slot
	NODE_ID FindNextNode(RuntimeContext& ctx, SLOT_ID fromSlot)
	{
		if (!ctx.graph) return 0;
		auto it = ctx.graph->edgeFromTo.find(fromSlot);
		if (it == ctx.graph->edgeFromTo.end()) return 0;
		const std::vector<SLOT_ID>& dests = it->second;
		for (SLOT_ID to : dests)
		{
			auto jt = ctx.graph->slotToNode.find(to);
			if (jt != ctx.graph->slotToNode.end())
				return jt->second;
		}
		return 0;
	}

	// Data Flow：输入值读取（pull model）
	Value GetInputValue(RuntimeContext& ctx, SLOT_ID inputSlotId)
	{
		if (!ctx.graph) return Value();
		if (inputSlotId >= ctx.graph->slots.size()) return Value();

		// 1) 查找反向边：inputSlot -> 上游 outputs
		auto it = ctx.graph->edgeToFrom.find(inputSlotId);
		if (it == ctx.graph->edgeToFrom.end() || it->second.empty())
		{
			// 2) 未连接：返回自身默认值
			return ctx.graph->slots[inputSlotId].value;
		}

		// 3) 已连接：取第一个上游输出槽
		const SLOT_ID fromSlotId = it->second.front();
		if (fromSlotId >= ctx.graph->slots.size())
		{
			return ctx.graph->slots[inputSlotId].value;
		}
		return ctx.graph->slots[fromSlotId].value;
	}

	// Advance from current node via its first exec output slot
	void AdvanceToNextNode(RuntimeContext& ctx)
	{
		// 检查上下文与图
		if (!ctx.graph) return;
		if (ctx.currentNodeId == 0) return;
		RuntimeNode* node = GetNodeById(*ctx.graph, ctx.currentNodeId);
		if (!node) return;

		// 遍历当前节点的所有输出槽
		// 仅对 Exec 类型且带有 active (Exec token) 的槽进行处理
		for (uint32_t i = 0; i < node->outputsCount; ++i)
		{
			uint32_t outIndex = node->outputsBegin + i;
			RuntimeSlot& outSlot = ctx.graph->slots[outIndex];
			SLOT_ID outSlotId = outSlot.id;

			// 只处理 Exec 类型的输出槽
			if (outSlot.type != SlotType::Exec) continue;

			// 如果该槽上没有 exec token 则跳过
			if (!ConsumeExec(ctx, outSlotId)) continue;

			// 查找该输出槽对应的所有目标槽，并把对应的目标节点推入 execStack
			auto it = ctx.graph->edgeFromTo.find(outSlotId);
			if (it == ctx.graph->edgeFromTo.end()) continue;
			const std::vector<SLOT_ID>& dests = it->second;
			for (SLOT_ID destSlot : dests)
			{
				auto jt = ctx.graph->slotToNode.find(destSlot);
				if (jt == ctx.graph->slotToNode.end()) continue;
				NODE_ID destNode = jt->second;
				// 把目标节点加入执行栈（支持多目标并行/分支）
				ctx.execStack.push_back(destNode);
			}
		}

		// 清理当前节点标识（当前节点执行完毕）
		ctx.currentNodeId = 0;
	}

	// Advance to next node by node id (if nextId == 0, end)
	void AdvanceToNextNode(RuntimeContext& ctx, NODE_ID nextId)
	{
		if (!ctx.graph) return;
		if (nextId == 0)
		{
			ctx.currentNodeId = 0;
			return;
		}
		ctx.currentNodeId = nextId;
	}

}
