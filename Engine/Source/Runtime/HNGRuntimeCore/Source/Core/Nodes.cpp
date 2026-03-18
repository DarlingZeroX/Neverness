/*
* This source file is part of VisionGal, the Visual Novel Engine
*
* For the latest information, see https://darlingzerox.github.io/VisionGalDoc/
* GitHub page: https://github.com/DarlingZeroX/VisionGal
*
* Copyright (c) 2025-present 梦旅缘心
*
* See the LICENSE file in the project root for details.
*/

#pragma once
#include <cstdint>
#include "Core/Nodes.h"

namespace Horizon::NodeGraphRuntime
{
	ExecResult EntryNodeExecute(RuntimeContext& ctx, NODE_ID nodeIndex)
	{
		if (!ctx.graph) return ExecResult::Finished;
		RuntimeNode* node = GetNodeById(*ctx.graph, nodeIndex);
		if (!node) return ExecResult::Finished;
		// 激活所有 Exec 类型输出槽
		for (uint32_t i = 0; i < node->outputsCount; ++i) {
			uint32_t outIdx = node->outputsBegin + i;
			RuntimeSlot& slot = ctx.graph->slots[outIdx];
			if (slot.type == SlotType::Exec) {
				PushExec(ctx, slot.id);
			}
		}
		return ExecResult::Finished;
	}

    // Branch 节点执行函数：根据输入条件激活 True/False 路径
	ExecResult BranchNodeExecute(RuntimeContext& ctx, NODE_ID nodeIndex)
	{
		if (!ctx.graph) return ExecResult::Finished;
		RuntimeNode* node = GetNodeById(*ctx.graph, nodeIndex);
		if (!node) return ExecResult::Finished;
		if (node->inputsCount == 0) return ExecResult::Finished;

		// 读取条件：Branch 的第 0 个输入通常是 Exec(In)，Bool 条件在后续输入槽中
		bool cond = false;
		for (uint32_t i = 0; i < node->inputsCount; ++i)
		{
			RuntimeSlot& inSlot = ctx.graph->slots[node->inputsBegin + i];
			if (inSlot.type == SlotType::Bool)
			{
				cond = inSlot.value.AsBool();
				break;
			}
		}

		// 查找 True/False 输出槽
		SLOT_ID trueSlot = FindOutputSlot(*ctx.graph, *node, "True");
		SLOT_ID falseSlot = FindOutputSlot(*ctx.graph, *node, "False");

		SLOT_ID outSlot = cond ? trueSlot : falseSlot;
		if (outSlot != 0) PushExec(ctx, outSlot);
		return ExecResult::Finished;
	}

	// Entry 节点执行函数：无状态，直接推进到下一个节点
	//ExecResult EntryNodeExecute(RuntimeContext& ctx, NODE_ID nodeIndex)
	//{
	//	AdvanceToNextNode(ctx);
	//	return ExecResult::Finished;
	//}

	// Dialogue 节点执行函数：支持多行对白和 Flow 等待
	ExecResult DialogueNodeExecute(RuntimeContext& ctx, NODE_ID nodeIndex)
	{
		if (!ctx.graph) return ExecResult::Finished;
		// 获取节点对象
		RuntimeNode* node = GetNodeById(*ctx.graph, nodeIndex);
		if (!node) return ExecResult::Finished;

		// Text：从字符串输出槽读取对白（由编译器从 editor properties 写入）
		SLOT_ID textSlotId = FindOutputSlot(*ctx.graph, *node, "Text");
		if (textSlotId != 0)
		{
			if (textSlotId < ctx.graph->slots.size())
			{
				const RuntimeSlot& textSlot = ctx.graph->slots[textSlotId];
				if (textSlot.value.type == ValueType::String)
				{
					printf("Dialogue: %s\n", textSlot.value.AsString().c_str());
				}
			}
		}

		// Next：激活控制流输出
		SLOT_ID nextSlotId = FindOutputSlot(*ctx.graph, *node, "Next");
		if (nextSlotId != 0) PushExec(ctx, nextSlotId);
		return ExecResult::Finished;
	}

	// Delay 节点执行函数：非阻塞等待，支持多帧
	ExecResult DelayNodeExecute(RuntimeContext& ctx, NODE_ID nodeIndex)
	{
		if (!ctx.graph) return ExecResult::Finished;
		RuntimeNode* node = GetNodeById(*ctx.graph, nodeIndex);
		if (!node) return ExecResult::Finished;

		// 获取/创建节点状态
		DelayState& state = ctx.GetOrCreateState<DelayState>(nodeIndex);

		// 获取 deltaTime（需 ctx 提供，若无则默认 0.016f）
		float dt = 0.016f;
		auto it = ctx.variables.find("deltaTime");
		if (it != ctx.variables.end())
			dt = static_cast<float>(it->second.AsFloat());

		// 获取等待时长（duration），优先从第一个输入槽读取
		float duration = 1.0f;
		if (node->inputsCount > 0)
		{
			RuntimeSlot& slot = ctx.graph->slots[node->inputsBegin];
			if (slot.value.type == ValueType::Float)
				duration = static_cast<float>(slot.value.AsFloat());
			else if (slot.value.type == ValueType::Int)
				duration = static_cast<float>(slot.value.AsInt());
		}
		else if (ctx.variables.find("delayDuration") != ctx.variables.end())
		{
			duration = static_cast<float>(ctx.variables["delayDuration"].AsFloat());
		}

		// 累加时间
		state.time += dt;

		// 判断是否完成
		if (state.time < duration)
		{
			return ExecResult::Running;
		}
		else
		{
			SLOT_ID nextSlotId = FindOutputSlot(*ctx.graph, *node, "Next");
			if (nextSlotId != 0) PushExec(ctx, nextSlotId);
			return ExecResult::Finished;
		}
	}
}
