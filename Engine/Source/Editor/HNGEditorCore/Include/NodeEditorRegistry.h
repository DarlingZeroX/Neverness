/*
* This source file is part of VisionGal, the Visual Novel Engine
*
* For the latest information, see https://darlingzerox.github.io/VisionGalDoc/
* GitHub page: https://github.com/DarlingZeroX/VisionGal
*/

#pragma once

#include <unordered_map>
#include <vector>

#include "../HNGEditorCoreConfig.h"
#include "NodeEditorMeta.h"

namespace Horizon::NodeGraphEditor
{
	class HNG_EDITOR_CORE_API NodeEditorRegistry
	{
	public:
		void Register(const NodeEditorMeta& meta)
		{
			metas[meta.type] = meta;
		}

		const NodeEditorMeta* Get(NodeGraphRuntime::NodeType type) const
		{
			auto it = metas.find(type);
			return (it != metas.end()) ? &it->second : nullptr;
		}

		// 返回全部元数据（指针稳定性：前提是注册完成后不再频繁触发 rehash）
		std::vector<const NodeEditorMeta*> GetAll() const
		{
			std::vector<const NodeEditorMeta*> out;
			out.reserve(metas.size());
			for (const auto& kv : metas)
				out.push_back(&kv.second);
			return out;
		}

	private:
		struct NodeTypeHash
		{
			size_t operator()(NodeGraphRuntime::NodeType t) const noexcept
			{
				using U = std::underlying_type_t<NodeGraphRuntime::NodeType>;
				return std::hash<U>{}(static_cast<U>(t));
			}
		};

		std::unordered_map<NodeGraphRuntime::NodeType, NodeEditorMeta, NodeTypeHash> metas;
	};
}

