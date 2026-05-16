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
#include "../HNodeGraphConfig.h"
#include "Types.h"
#include "Value.h"
#include <NNCore/Interface/HConfig.h>
#include <NNCore/Interface/HCoreTypes.h>
#include <unordered_map>
#include <memory>

namespace NN::Core::NodeGraphRuntime
{
	struct RuntimeContext;
	struct CompiledExpression;

	// 执行函数返回 ExecResult 表示节点当前是否仍在运行（ExecResult 在 Types.h 中定义）
	using NodeExecuteFn = ExecResult(*)(RuntimeContext& ctx, NODE_ID nodeIndex);

	// RuntimeSlot 表示节点的输入/输出槽
	// 说明：
	// - 每个槽由唯一的 SLOT_ID 标识（SLOT_ID == RuntimeGraph::slots 的索引）
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
		NodeTypeId typeId;

		// 输入输出槽的索引范围（在 graph.slots 中）
		uint32_t inputsBegin;
		uint32_t inputsCount;
		uint32_t outputsBegin;
		uint32_t outputsCount;

		// 执行函数（由注册表提供）
		NodeExecuteFn execute;

		// 输出槽快速索引表（O(1) FindOutputSlot）
		// 说明：
		// - key   : slot.name（例如 "Next"/"True"/"Text"）
		// - value : SLOT_ID（注意：SLOT_ID == RuntimeGraph::slots 的索引）
		// - 该表由编译阶段填充（GraphCompiler），运行时只读
		std::unordered_map<std::string, SLOT_ID> outputSlotMap;

		// 节点内表达式缓存表（使用 shared_ptr<CompiledExpression> 支持前向声明）
		// - key   : 表达式 key（通常为属性名，例如 "condition"/"value"）
		// - value : 预编译表达式结构（CompiledExpression）的共享指针
		// 说明：
		// - CompiledExpression 在 ExpressionEvaluator.h 中给出完整定义；
		//   这里使用 shared_ptr 可以只依赖前向声明，避免头文件循环依赖问题。
		std::unordered_map<std::string, std::shared_ptr<CompiledExpression>> expressionCache;

		// 构造
		RuntimeNode() noexcept
			: id(0), typeId(NodeTypeIdInvalid), inputsBegin(0), inputsCount(0), outputsBegin(0), outputsCount(0), execute(nullptr)
		{}
	};
}
