/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include <cstdint>
#include <functional>
#include <vector>

namespace VisionGal::Editor::ReactiveCore
{
	using DerivedStateId = uint32_t;

	/// 单线程有向依赖图：Invalidate 标记脏，FlushDirty 按拓扑顺序调用 Compute。
	class DerivedStateGraph
	{
	public:
		using ComputeFn = std::function<void()>;

		void Clear();
		void RegisterNode(DerivedStateId id, std::vector<DerivedStateId> dependsOn, ComputeFn compute);
		void Invalidate(DerivedStateId id);
		void InvalidateAll();
		[[nodiscard]] bool IsDirty(DerivedStateId id) const;
		void FlushDirty();

	private:
		struct Node
		{
			DerivedStateId id = 0;
			std::vector<DerivedStateId> dependsOn;
			std::vector<DerivedStateId> dependents;
			ComputeFn compute;
			bool dirty = false;
		};

		std::vector<Node> m_nodes;
		std::vector<DerivedStateId> m_topoOrder;

		void RebuildTopo();
		void RebuildDependentsList();
		[[nodiscard]] int FindIndex(DerivedStateId id) const;
	};
}
