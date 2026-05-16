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

namespace NN::Core::NodeGraphRuntime
{
	// ----------------------------
	// NodeType 注册机制：
	// - TypeId: 由 NodeRegistry 分配（NodeTypeId = uint16_t）
	// - name -> typeId 与 typeId -> name 维护映射
	// - meta(typeId) -> NodeMeta：包含执行函数与 pins 描述
	// ----------------------------
	class NodeRegistry
	{
	public:
		NodeRegistry()
		{
			// 内置类型初始化：确保 GraphCompiler/Editor 可以稳定找到关键类型
			// 注意：这里只注册“内置类型名”，不强依赖业务语义。
			// 对应 execute/pins 元数据仍需在上层（Editor/模块）通过 Register(NodeMeta) 注册。
			// 保留 id=0 作为 Invalid
			nameToId.emplace("Invalid", NodeTypeIdInvalid);
			idToName.emplace(NodeTypeIdInvalid, "Invalid");
			RegisterType("Entry");
			RegisterType("Dialogue");
			RegisterType("Delay");
			RegisterType("Branch");
			RegisterType("SetVariable");
			RegisterType("GetVariable");
			RegisterType("Condition");
			RegisterType("Custom0");
			RegisterType("Custom1");
		}

		// 分配/返回 typeId
		NodeTypeId RegisterType(const std::string& name)
		{
			if (name.empty()) return NodeTypeIdInvalid;
			if (name == "Invalid") return NodeTypeIdInvalid;
			auto it = nameToId.find(name);
			if (it != nameToId.end()) return it->second;

			NodeTypeId next = nextTypeId++;
			nameToId.emplace(name, next);
			idToName.emplace(next, name);
			return next;
		}

		const std::string& GetTypeName(NodeTypeId id) const
		{
			static const std::string empty{};
			auto it = idToName.find(id);
			return (it != idToName.end()) ? it->second : empty;
		}

		NodeTypeId FindType(const std::string& name) const
		{
			auto it = nameToId.find(name);
			return (it != nameToId.end()) ? it->second : NodeTypeIdInvalid;
		}

		// 注册节点元数据：包含执行函数与槽元数据
		void Register(const NodeMeta& meta)
		{
			metas[meta.typeId] = meta;
		}

		const NodeMeta* GetMeta(NodeTypeId typeId) const
		{
			auto it = metas.find(typeId);
			return (it != metas.end()) ? &it->second : nullptr;
		}

		NodeExecuteFn Get(NodeTypeId typeId) const
		{
			auto it = metas.find(typeId);
			return (it != metas.end()) ? it->second.execute : nullptr;
		}

	private:
		std::unordered_map<std::string, NodeTypeId> nameToId;
		std::unordered_map<NodeTypeId, std::string> idToName;
		std::unordered_map<NodeTypeId, NodeMeta> metas;

		NodeTypeId nextTypeId = 1; // 0 保留为 invalid
	};

}
