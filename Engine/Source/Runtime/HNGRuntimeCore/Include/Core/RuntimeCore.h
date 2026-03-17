/*
 * 本源文件属于 VisionGal 视觉小说引擎的运行时节点图模块
 *
 * 最新信息请见: https://darlingzerox.github.io/VisionGalDoc/
 * GitHub: https://github.com/DarlingZeroX/VisionGal
 *
 * 版权所有 (c) 2025-present 梦旅缘心
 *
 * 详情请参阅项目根目录下的 LICENSE 文件。
 */

#pragma once
#include "../Config.h"
#include "Types.h"
#include "Value.h"
#include <HCore/Interface/HConfig.h>
#include <HCore/Interface/HCoreTypes.h>

namespace Horizon::NodeGraphRuntime
{
	struct RuntimeContext;

	// 执行函数返回 ExecResult 表示节点当前是否仍在运行（ExecResult 在 Types.h 中定义）
	using NodeExecuteFn = ExecResult(*)(RuntimeContext& ctx, NODE_ID nodeIndex);

	// RuntimeSlot 表示节点的输入/输出槽
	// 说明：
	// - 每个槽由唯一的 SLOT_ID 标识（由 MakeSlotId 生成）
	// - name 字段用于标识 Exec 输出（例如 "True"/"False"/"Next"），也可用于数据槽命名
	// - type 区分槽的数据类型或是否为 Exec 控制流槽
	struct RuntimeSlot
	{
		SLOT_ID id;
		std::string name;
		SlotType type;
		bool active;
		Value value;

		RuntimeSlot() noexcept : id(0), type(SlotType::Int), active(false), value() {}
		RuntimeSlot(SLOT_ID i, SlotType t) noexcept : id(i), type(t), active(false), value() {}
	};

	// RuntimeEdge 表示图中的连接：从某个输出槽连接到另一个槽（通常是目标节点的输入槽）
	struct RuntimeEdge
	{
		SLOT_ID from;
		SLOT_ID to;
		RuntimeEdge() noexcept : from(0), to(0) {}
		RuntimeEdge(SLOT_ID f, SLOT_ID t) noexcept : from(f), to(t) {}
	};

	// RuntimeNode：POD 风格
	struct RuntimeNode
	{
		NODE_ID id;
		NodeType type;

		// 输入输出槽的索引范围（在 graph.slots 中）
		uint32_t inputsBegin;
		uint32_t inputsCount;
		uint32_t outputsBegin;
		uint32_t outputsCount;

		// 执行函数（由注册表提供）
		NodeExecuteFn execute;

		// 构造
		RuntimeNode() noexcept
			: id(0), type(NodeType::Invalid), inputsBegin(0), inputsCount(0), outputsBegin(0), outputsCount(0), execute(nullptr)
		{}
	};
}
