/*
* 运行时 NodeGraph 实现（无虚函数、POD 风格、函数指针驱动）
* 提供基础的数据结构：Value, RuntimeSlot, RuntimeEdge, RuntimeNode, RuntimeGraph, RuntimeContext
*/

#pragma once

#include <any>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <cstdint>
#include <functional>
#include <algorithm>
#include "RuntimeGraph.h"

namespace Horizon::NodeGraphRuntime
{
	// 将执行 token 放入指定槽（用于控制流到达某个槽）
	H_NODE_GRAPH_API void PushExec(RuntimeContext& ctx, SLOT_ID slotId);

	// 如果槽上存在 exec token，则消费并返回 true；否则返回 false
	H_NODE_GRAPH_API bool ConsumeExec(RuntimeContext& ctx, SLOT_ID slotId);

	// 对于给定的输出槽，查找 edgeFromTo 列表并返回首个对应的目标节点 ID（如无返回 0）
	H_NODE_GRAPH_API NODE_ID FindNextNode(RuntimeContext& ctx, SLOT_ID fromSlot);

	// 将当前节点的所有输出槽的目标节点加入执行栈（execStack），并在必要时设置 exec token
	H_NODE_GRAPH_API void AdvanceToNextNode(RuntimeContext& ctx);

	// 直接将 currentNodeId 设置为指定 nextId（0 表示结束）
	H_NODE_GRAPH_API void AdvanceToNextNode(RuntimeContext& ctx, NODE_ID nextId);

	// ExecuteGraph: 使用 execStack 驱动整个图的执行主循环
	// 语义：弹出 execStack 的节点，执行其 NodeExecuteFn。
	// - 如果节点返回 Running，则重新入栈以便下一轮继续执行
	// - 如果节点返回 Finished，则调用 AdvanceToNextNode 推动控制流到下游节点
	// 为避免死循环，内部包含最大执行步数保护
	H_NODE_GRAPH_API void ExecuteGraph(RuntimeContext& ctx);

	// ----------------------------
	// Data Flow：输入值读取（pull model）
	//
	// GetInputValue 的设计目标：
	// - 让节点“读取输入值”时不再直接访问 ctx.graph->slots[...]（禁止）
	// - 输入值来自连接的上游输出槽（通过 edgeToFrom 反查）
	//
	// 行为：
	// 1) 查找 edgeToFrom[inputSlotId]
	// 2) 若没有连接：返回当前 input 槽自身的 slot.value（作为默认值）
	// 3) 若有连接：取第一个上游槽（fromSlotId），返回上游槽的 slot.value
	//
	// 说明：
	// - 当前版本不做 merge/聚合，只取第一条连接（后续可扩展）
	// - SLOT_ID == slots 索引，因此可以直接 slots[slotId] O(1) 访问
	// ----------------------------
	H_NODE_GRAPH_API Value GetInputValue(RuntimeContext& ctx, SLOT_ID inputSlotId);

	// RuntimeContext: 执行时上下文
	// 说明：
	// - currentNodeId: 当前正在执行的节点 ID（本实现主要使用 execStack，为兼容保留）
	// - variables: 全局运行时变量表（key->Value），供节点间传递数据或被断言测试
	// - execStack: 执行栈，Push / Pop 节点 ID 以驱动执行流（采用 LIFO，此处表示微任务执行）
	// - jumped: 标记节点在执行期间是否主动跳转（供高级控制流使用）
	// - graph: 当前执行的 RuntimeGraph 指针
	// - nodeStates: 每个节点的本地状态存储（通过 GetOrCreateState<T> 访问）
	struct RuntimeContext
	{
		NODE_ID currentNodeId;
		std::unordered_map<std::string, Value> variables;
		std::vector<NODE_ID> execStack; // execution stack of node ids
		std::vector<NODE_ID> executedNodes;
		std::unordered_set<NODE_ID> executedNodeSet;

		// deltaTime：每帧时间步长（秒）
		// 用途：Delay、动画、自动播放对话等需要“随时间推进”的节点
		float deltaTime;
		bool jumped;

		// pointer to graph being executed
		RuntimeGraph* graph;

		// per-node状态存储（任意类型）
		std::unordered_map<NODE_ID, std::any> nodeStates;

		RuntimeContext() noexcept : currentNodeId(0), deltaTime(0.0f), jumped(false), graph(nullptr) {}

		bool WasNodeExecuted(NODE_ID id) const
		{
			return executedNodeSet.find(id) != executedNodeSet.end();
		}

		// 获取并创建节点本地状态对象（若不存在则默认构造）
		template<typename T>
		T& GetOrCreateState(NODE_ID nodeId)
		{
			auto it = nodeStates.find(nodeId);
			if (it == nodeStates.end())
			{
				nodeStates[nodeId] = T();
				it = nodeStates.find(nodeId);
			}
			return std::any_cast<T&>(it->second);
		}
	};

}
