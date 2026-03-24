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
#include "Nodes.h"

#include <iostream>

#include "ExpressionEvaluator.h"

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
		std::cout << "EntryNodeExecute: Activated " << node->outputsCount << " Exec outputs." << std::endl;
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
			const SLOT_ID inputSlotId = static_cast<SLOT_ID>(node->inputsBegin + i);
			// ✅ 必须使用 GetInputValue 读取输入（数据来自上游连接）
			Value v = GetInputValue(ctx, inputSlotId);
			// 仅对 Bool 类型输入槽进行条件读取
			if (ctx.graph->slots[inputSlotId].type == SlotType::Bool)
			{
				cond = v.AsBool();
				break;
			}
		}

		// 查找 True/False 输出槽
		SLOT_ID trueSlot = FindOutputSlot(*ctx.graph, *node, "True");
		SLOT_ID falseSlot = FindOutputSlot(*ctx.graph, *node, "False");

		SLOT_ID outSlot = cond ? trueSlot : falseSlot;
		if (outSlot != 0) PushExec(ctx, outSlot);

		std::cout << "BranchNodeExecute: Condition = " << cond << ", Activated " << (cond ? "True" : "False") << " output." << std::endl;
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

		// ------------------------------------------------------------------
		// 多行对白状态结构（按节点持久化）：
		// - currentLine : 当前正在显示的行号（从 0 开始）
		// - lines       : 由 Text 槽内容按 '\n' 拆分出的所有对白行
		// - initialized : 是否已经从 Text 槽完成初始化拆分
		//
		// 状态通过 RuntimeContext::GetOrCreateState<T> 绑定到 nodeIndex，
		// 因此在多次调用 DialogueNodeExecute 之间会自动保持。
		// ------------------------------------------------------------------
		struct DialogueState
		{
			int currentLine = 0;
			std::vector<std::string> lines;
			bool initialized = false;
			bool shownThisLine = false;
		};

		DialogueState& state = ctx.GetOrCreateState<DialogueState>(nodeIndex);

		// Text：从字符串输出槽读取完整对白文本（由编译器从 editor properties["text"] 写入）
		SLOT_ID textSlotId = FindOutputSlot(*ctx.graph, *node, "Text");
		if (!state.initialized && textSlotId != 0)
		{
			if (textSlotId < ctx.graph->slots.size())
			{
				const RuntimeSlot& textSlot = ctx.graph->slots[textSlotId];
				if (textSlot.value.type == ValueType::String)
				{
					const std::string& fullText = textSlot.value.AsString();

					// 初始化：按 '\n' 拆分多行文本
					state.lines.clear();
					std::string current;
					for (char c : fullText)
					{
						if (c == '\n')
						{
							state.lines.push_back(current);
							current.clear();
						}
						else
						{
							current.push_back(c);
						}
					}
					// 收尾：最后一行（无论是否以 '\n' 结尾）
					if (!current.empty() || fullText.empty())
					{
						state.lines.push_back(current);
					}

					state.currentLine = 0;
					state.initialized = true;
				}
			}
		}

		// 若尚未成功初始化（没有 Text 槽或值为空），则直接结束
		if (!state.initialized || state.lines.empty())
			return ExecResult::Finished;

		// 若当前行还在有效范围内：展示本行对白，并等待玩家输入推进
		if (state.currentLine < static_cast<int>(state.lines.size()))
		{
			const std::string& line = state.lines[static_cast<size_t>(state.currentLine)];
			// 仅在“本行尚未显示过”时打印一次，避免 Running 状态下重复刷屏
			if (!state.shownThisLine && !line.empty())
			{
				printf("Dialogue: %s\n", line.c_str());
				state.shownThisLine = true;
			}

			// 等待玩家输入：
			// 约定：外部逻辑在玩家点击“下一句”时，将 ctx.variables["Next"] 置为 true。
			bool readyNext = false;
			auto itNext = ctx.variables.find("Next");
			if (itNext != ctx.variables.end() && itNext->second.type == ValueType::Bool)
			{
				readyNext = itNext->second.AsBool();
			}

			if (!readyNext)
			{
				// 未收到“下一句”指令：保持当前行，返回 Running 让 ExecuteGraph 继续在该节点上轮询
				return ExecResult::Running;
			}

			// 已确认推进：消费 Next 标记并前进到下一行
			ctx.variables["Next"] = Value(false);
			++state.currentLine;
			state.shownThisLine = false;

			// 仍有后续行：继续保持 Running 以便下一帧再次执行本节点
			if (state.currentLine < static_cast<int>(state.lines.size()))
			{
				return ExecResult::Running;
			}
		}

		// 所有行已播放完毕：激活 Next 控制流输出，结束节点
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

		// 获取 deltaTime（秒）
		// 说明：由 RuntimeContext 提供，避免通过 variables 做字符串查找
		// 若未设置（==0），这里提供一个安全的默认值（约 60fps）
		float dt = (ctx.deltaTime > 0.0f) ? ctx.deltaTime : 0.016f;

		// 获取等待时长（duration），优先从第一个输入槽读取
		float duration = 1.0f;
		if (node->inputsCount > 0)
		{
			const SLOT_ID inputSlotId = static_cast<SLOT_ID>(node->inputsBegin);
			Value v = GetInputValue(ctx, inputSlotId);
			if (v.type == ValueType::Float)
				duration = static_cast<float>(v.AsFloat());
			else if (v.type == ValueType::Int)
				duration = static_cast<float>(v.AsInt());
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

	// ------------------------------------------------------------------
	// SetVariable 节点：执行表达式并写入全局变量表
	//
	// 设计意图：
	// - 为图形化节点图提供“赋值语句”，形如：
	//     name   = "hp"
	//     value  = "hp + 10"
	//   运行时求值后相当于：ctx.variables["hp"] = EvaluateExpression("hp + 10", ctx);
	//
	// 槽约定：
	// - 输出 "Name"      : string，存放变量名
	// - 输出 "Expression": string，存放表达式源码
	// - 输出 "Next"      : Exec，赋值完成后激活的控制流输出
	// ------------------------------------------------------------------
	ExecResult SetVariableNodeExecute(RuntimeContext& ctx, NODE_ID nodeIndex)
	{
		if (!ctx.graph) return ExecResult::Finished;
		RuntimeNode* node = GetNodeById(*ctx.graph, nodeIndex);
		if (!node) return ExecResult::Finished;

		// 读取 Name 槽（变量名）
		SLOT_ID nameSlotId = FindOutputSlot(*ctx.graph, *node, "Name");
		std::string varName;
		if (nameSlotId != 0 && nameSlotId < ctx.graph->slots.size())
		{
			const RuntimeSlot& nameSlot = ctx.graph->slots[nameSlotId];
			if (nameSlot.value.type == ValueType::String)
				varName = nameSlot.value.AsString();
		}
		if (varName.empty())
		{
			// 未配置变量名：直接结束
			return ExecResult::Finished;
		}

		// 读取 Expression 槽（表达式源码）
		SLOT_ID exprSlotId = FindOutputSlot(*ctx.graph, *node, "Expression");
		std::string expr;
		if (exprSlotId != 0 && exprSlotId < ctx.graph->slots.size())
		{
			const RuntimeSlot& exprSlot = ctx.graph->slots[exprSlotId];
			if (exprSlot.value.type == ValueType::String)
			 expr = exprSlot.value.AsString();
		}

		// 评价表达式并写入变量表
		if (!expr.empty())
		{
			Value result = EvaluateExpression(expr, ctx);
			ctx.variables[varName] = result;
		}

		// 推进控制流到 Next
		SLOT_ID nextSlotId = FindOutputSlot(*ctx.graph, *node, "Next");
		if (nextSlotId != 0) PushExec(ctx, nextSlotId);
		return ExecResult::Finished;
	}

	// ------------------------------------------------------------------
	// GetVariable 节点：从全局变量表读取变量值并输出到数据槽
	//
	// 设计意图：
	// - 作为 SetVariable 的“反向操作”，让后续节点可以把变量当作普通输入使用
	//
	// 槽约定：
	// - 输出 "Name" : string，存放变量名
	// - 输出 "Value": 任意类型（SlotType 可设为 String 以便在 Editor 中显示），
	//                 实际运行时将 Value 整体写入 RuntimeSlot.value
	// - 输出 "Next" : Exec，读取完成后激活的控制流输出
	// ------------------------------------------------------------------
	ExecResult GetVariableNodeExecute(RuntimeContext& ctx, NODE_ID nodeIndex)
	{
		if (!ctx.graph) return ExecResult::Finished;
		RuntimeNode* node = GetNodeById(*ctx.graph, nodeIndex);
		if (!node) return ExecResult::Finished;

		// 读取 Name 槽
		SLOT_ID nameSlotId = FindOutputSlot(*ctx.graph, *node, "Name");
		std::string varName;
		if (nameSlotId != 0 && nameSlotId < ctx.graph->slots.size())
		{
			const RuntimeSlot& nameSlot = ctx.graph->slots[nameSlotId];
			if (nameSlot.value.type == ValueType::String)
				varName = nameSlot.value.AsString();
		}
		if (varName.empty())
			return ExecResult::Finished;

		// 从变量表读取值
		Value value;
		{
			auto it = ctx.variables.find(varName);
			if (it != ctx.variables.end())
				value = it->second;
		}

		// 写入 Value 输出槽
		SLOT_ID valueSlotId = FindOutputSlot(*ctx.graph, *node, "Value");
		if (valueSlotId != 0 && valueSlotId < ctx.graph->slots.size())
		{
			RuntimeSlot& out = ctx.graph->slots[valueSlotId];
			out.value = value;
		}

		// 推进控制流到 Next
		SLOT_ID nextSlotId = FindOutputSlot(*ctx.graph, *node, "Next");
		if (nextSlotId != 0) PushExec(ctx, nextSlotId);
		return ExecResult::Finished;
	}

	// ------------------------------------------------------------------
	// Condition 节点：基于表达式的条件分支（升级版 Branch）
	//
	// 设计意图：
	// - 替代传统“Bool 输入 + True/False 输出”的 Branch 设计，
	//   直接在 Editor 中写逻辑表达式：
	//     condition = "hp > 10 && hasKey == true"
	//
	// 槽约定：
	// - 输出 "Condition" : string，存放条件表达式
	// - 输出 "True"      : Exec，条件为 true 时激活
	// - 输出 "False"     : Exec，条件为 false 时激活
	// ------------------------------------------------------------------
	ExecResult ConditionNodeExecute(RuntimeContext& ctx, NODE_ID nodeIndex)
	{
		if (!ctx.graph) return ExecResult::Finished;
		RuntimeNode* node = GetNodeById(*ctx.graph, nodeIndex);
		if (!node) return ExecResult::Finished;

		// 读取 Condition 表达式
		SLOT_ID condSlotId = FindOutputSlot(*ctx.graph, *node, "Condition");
		std::string condExpr;
		if (condSlotId != 0 && condSlotId < ctx.graph->slots.size())
		{
			const RuntimeSlot& condSlot = ctx.graph->slots[condSlotId];
			if (condSlot.value.type == ValueType::String)
				condExpr = condSlot.value.AsString();
		}
		if (condExpr.empty())
		{
			return ExecResult::Finished;
		}

		// 求值表达式
		Value res = EvaluateExpression(condExpr, ctx);
		bool cond = res.AsBool();

		// 根据结果激活 True / False 输出
		const char* outName = cond ? "True" : "False";
		SLOT_ID outSlotId = FindOutputSlot(*ctx.graph, *node, outName);
		if (outSlotId != 0)
			PushExec(ctx, outSlotId);

		return ExecResult::Finished;
	}
}
