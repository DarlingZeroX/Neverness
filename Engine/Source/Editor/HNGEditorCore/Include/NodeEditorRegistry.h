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
#include "../Interface/NodeEditorMeta.h"

namespace Horizon::NodeGraphEditor
{
	class HNG_EDITOR_CORE_API NodeEditorRegistry
	{
	public:
		void Register(const NodeEditorMeta& meta)
		{
			metas[meta.typeId] = meta;
		}

		const NodeEditorMeta* Get(NodeGraphRuntime::NodeTypeId typeId) const
		{
			auto it = metas.find(typeId);
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
		std::unordered_map<NodeGraphRuntime::NodeTypeId, NodeEditorMeta> metas;
	};
}

