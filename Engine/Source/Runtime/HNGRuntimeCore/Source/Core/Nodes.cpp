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
		AdvanceToNextNode(ctx);
		return ExecResult::Finished;
	}

    // Branch 节点执行函数：根据输入条件激活 True/False 路径
	ExecResult BranchNodeExecute(RuntimeContext& ctx, NODE_ID nodeIndex)
	{
		RuntimeNode* node = GetNodeById(*ctx.graph, nodeIndex);
		if (!node) return ExecResult::Finished;
		if (node->inputsCount == 0) return ExecResult::Finished;

		// 读取条件（第一个输入槽）
		RuntimeSlot& condSlot = ctx.graph->slots[node->inputsBegin];
		bool cond = condSlot.value.AsBool();

		// 查找 True/False 输出槽
		SLOT_ID trueSlot = FindOutputSlot(*ctx.graph, *node, "True");
		SLOT_ID falseSlot = FindOutputSlot(*ctx.graph, *node, "False");

		SLOT_ID outSlot = cond ? trueSlot : falseSlot;
		if (outSlot != 0)
		{
			PushExec(ctx, outSlot);
			// 只激活对应路径
			NODE_ID nextId = FindNextNode(ctx, outSlot);
			AdvanceToNextNode(ctx, nextId);
		}
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
		// 获取节点对象
		RuntimeNode* node = GetNodeById(*ctx.graph, nodeIndex);
		if (!node) return ExecResult::Finished;

		// 获取/创建节点状态
		DialogueState& state = ctx.GetOrCreateState<DialogueState>(nodeIndex);

		int totalLines = static_cast<int>(node->outputsCount);
		if (totalLines == 0) {
			AdvanceToNextNode(ctx);
			return ExecResult::Finished;
		}

		// 如果处于等待状态，模拟 Flow 完成
		if (state.waiting) {
			// 这里可扩展为条件检查或异步事件
			state.waiting = false;
			state.currentLine++;
		}

		// 如果还有对白行
		if (state.currentLine < totalLines) {
			// 输出当前行文本
			const RuntimeSlot& lineSlot = ctx.graph->slots[node->outputsBegin + state.currentLine];
			if (lineSlot.value.type == ValueType::String) {
				printf("Dialogue: %s\n", lineSlot.value.AsString().c_str());
			}

			// 检查当前行是否有 Flow（可用槽 name 或 value 标记）
			bool hasFlow = (lineSlot.name == "Flow");
			if (hasFlow) {
				state.waiting = true;
				return ExecResult::Running;
			} else {
				state.currentLine++;
				// 继续下一行（本帧只输出一行）
				return ExecResult::Running;
			}
		}

		// 所有行完成，推进到下一个节点
		AdvanceToNextNode(ctx);
		return ExecResult::Finished;
	}

	// Delay 节点执行函数：非阻塞等待，支持多帧
	ExecResult DelayNodeExecute(RuntimeContext& ctx, NODE_ID nodeIndex)
	{
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
			AdvanceToNextNode(ctx);
			return ExecResult::Finished;
		}
	}
}
