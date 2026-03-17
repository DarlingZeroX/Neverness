/*
* 运行时 NodeGraph 实现（无虚函数、POD 风格、函数指针驱动）
* 提供基础的数据结构：Value, RuntimeSlot, RuntimeEdge, RuntimeNode, RuntimeGraph, RuntimeContext
*/

#pragma once
#include <unordered_map>
#include "Value.h"
#include "Types.h"
#include "RuntimeCore.h"

namespace Horizon::NodeGraphRuntime
{
	// 注册机制：简单注册表，把 NodeType 映射到执行函数
	struct NodeRegistry
	{
		std::unordered_map<NodeType, NodeExecuteFn> table;

		void Register(NodeType t, NodeExecuteFn fn) { table[t] = fn; }
		NodeExecuteFn Get(NodeType t) const
		{
			auto it = table.find(t);
			return (it != table.end()) ? it->second : nullptr;
		}
	};

}
