/*
* 运行时 NodeGraph 实现（无虚函数、POD 风格、函数指针驱动）
* 提供基础的数据结构：Value, RuntimeSlot, RuntimeEdge, RuntimeNode, RuntimeGraph, RuntimeContext
*/

#pragma once
#include <unordered_map>
#include <type_traits>
#include "NodeMeta.h"
#include "../Interface/Value.h"
#include "../Interface/Types.h"
#include "../Interface/RuntimeCore.h"

namespace Horizon::NodeGraphRuntime
{
	struct NodeTypeHash
	{
		size_t operator()(NodeType t) const noexcept
		{
			using U = std::underlying_type_t<NodeType>;
			return std::hash<U>{}(static_cast<U>(t));
		}
	};

	// 注册机制：NodeType -> NodeMeta（包含执行函数与槽元数据）
	class NodeRegistry
	{
	public:
		void Register(const NodeMeta& meta)
		{
			metas[meta.type] = meta;
		}

		const NodeMeta* GetMeta(NodeType type) const
		{
			auto it = metas.find(type);
			return (it != metas.end()) ? &it->second : nullptr;
		}

		NodeExecuteFn Get(NodeType t) const
		{
			auto it = metas.find(t);
			return (it != metas.end()) ? it->second.execute : nullptr;
		}

	private:
		std::unordered_map<NodeType, NodeMeta, NodeTypeHash> metas;
	};

}
