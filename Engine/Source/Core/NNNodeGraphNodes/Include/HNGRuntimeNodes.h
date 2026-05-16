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

// HNGRuntimeNodes：运行时“节点实现库”
// - 只提供各类节点的 Execute 函数实现
// - 依赖 HNGRuntimeCore（RuntimeContext/RuntimeGraph/Value/NodeRegistry 等）

#include <cstdint>
#include "../HNGRuntimeNodesConfig.h"
#include "../../HNGRuntimeCore/Interface/RuntimeCore.h"
#include "../../HNGRuntimeCore/Include/RuntimeContext.h"

namespace NN::Core::NodeGraphRuntime
{
	// Entry 节点执行函数：无状态，直接推进到下一个节点
	H_NG_RUNTIME_NODES_API ExecResult EntryNodeExecute(RuntimeContext& ctx, NODE_ID nodeIndex);

	// Dialogue 节点状态：用于存储当前行和等待状态
	struct DialogueState
	{
		int currentLine = 0;
		bool waiting = false;
	};

	// Dialogue 节点执行函数：支持多行对白和 Flow 等待
	H_NG_RUNTIME_NODES_API ExecResult DialogueNodeExecute(RuntimeContext& ctx, NODE_ID nodeIndex);

	// Branch 节点执行函数：根据输入条件激活 True/False 路径
	H_NG_RUNTIME_NODES_API ExecResult BranchNodeExecute(RuntimeContext& ctx, NODE_ID nodeIndex);

	// SetVariable 节点执行函数：
	// - 从配置的表达式字符串中求值，并将结果写入 ctx.variables[name]
	// - 典型用法：在 Editor 中设置 name="hp", value="100" 或 "hp + 10"
	H_NG_RUNTIME_NODES_API ExecResult SetVariableNodeExecute(RuntimeContext& ctx, NODE_ID nodeIndex);

	// GetVariable 节点执行函数：
	// - 从 ctx.variables[name] 中读取变量值，写入本节点的 "Value" 输出槽
	// - 便于后续节点通过普通数据槽访问变量
	H_NG_RUNTIME_NODES_API ExecResult GetVariableNodeExecute(RuntimeContext& ctx, NODE_ID nodeIndex);

	// Condition 节点执行函数（升级版 Branch）：
	// - 从配置的条件表达式字符串中求值（结果为 bool）
	// - 若为 true：激活 "True" 控制流输出；否则激活 "False" 输出
	// - 典型用法：condition="hp > 10 && hasKey == true"
	H_NG_RUNTIME_NODES_API ExecResult ConditionNodeExecute(RuntimeContext& ctx, NODE_ID nodeIndex);

	// Delay 节点状态：用于存储累计等待时间
	struct DelayState
	{
		float time = 0.0f;
	};

	// Delay 节点执行函数：非阻塞等待，支持多帧
	H_NG_RUNTIME_NODES_API ExecResult DelayNodeExecute(RuntimeContext& ctx, NODE_ID nodeIndex);
}

