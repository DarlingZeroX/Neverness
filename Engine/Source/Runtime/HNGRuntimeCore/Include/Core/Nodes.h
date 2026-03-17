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
#include "RuntimeCore.h"
#include "RuntimeContext.h"

namespace Horizon::NodeGraphRuntime
{
    // Entry 节点执行函数：无状态，直接推进到下一个节点
	H_NODE_GRAPH_API ExecResult EntryNodeExecute(RuntimeContext& ctx, NODE_ID nodeIndex);

	// Dialogue 节点状态：用于存储当前行和等待状态
	struct DialogueState
	{
		int currentLine = 0;
		bool waiting = false;
	};

	// Dialogue 节点执行函数：支持多行对白和 Flow 等待
	H_NODE_GRAPH_API ExecResult DialogueNodeExecute(RuntimeContext& ctx, NODE_ID nodeIndex);

	// Branch 节点执行函数：根据输入条件激活 True/False 路径
	H_NODE_GRAPH_API ExecResult BranchNodeExecute(RuntimeContext& ctx, NODE_ID nodeIndex);

	// Delay 节点状态：用于存储累计等待时间
	struct DelayState
	{
		float time = 0.0f;
	};

	// Delay 节点执行函数：非阻塞等待，支持多帧
	H_NODE_GRAPH_API ExecResult DelayNodeExecute(RuntimeContext& ctx, NODE_ID nodeIndex);
}
