/*
* 运行时 NodeGraph 实现（无虚函数、POD 风格、函数指针驱动）
* 提供基础的数据结构：Value, RuntimeSlot, RuntimeEdge, RuntimeNode, RuntimeGraph, RuntimeContext
*/

#pragma once

#include <any>
#include <vector>
#include <string>
#include <unordered_map>
#include <cstdint>
#include <functional>
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
		bool jumped;

		// pointer to graph being executed
		RuntimeGraph* graph;

		// per-node状态存储（任意类型）
		std::unordered_map<NODE_ID, std::any> nodeStates;

		RuntimeContext() noexcept : currentNodeId(0), jumped(false), graph(nullptr) {}

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
