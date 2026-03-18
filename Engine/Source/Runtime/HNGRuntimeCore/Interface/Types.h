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

namespace Horizon::NodeGraphRuntime
{
	using NODE_ID = std::uint32_t;
	// SLOT_ID 直接等于 RuntimeGraph::slots 的索引（消除运行时 hash 查找）
	using SLOT_ID = std::uint32_t;

	// Slot 类型（Exec 控制流 或 数据槽）
	enum class SlotType : uint8_t
	{
		Exec = 0,
		Bool,
		Int,
		Float,
		String,
		Object // reserved
	};

	// 节点类型：由用户定义的枚举（引擎端需要扩展）
	enum class NodeType : uint16_t
	{
		Invalid = 0,
		Entry,
		Dialogue,
		Delay,
		Branch,
		Custom0,
		Custom1
	};

	// 节点执行函数指针签名
	// 执行函数接收上下文和节点索引（节点在 graph.nodes 向量中的索引）
	enum class ExecResult : uint8_t
	{
		Finished = 0,
		Running = 1
	};
}
