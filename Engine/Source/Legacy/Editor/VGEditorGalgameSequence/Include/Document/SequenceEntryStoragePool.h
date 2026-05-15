/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#pragma once

#include "ViewModels/SequenceEntryViewModel.h"

#include <vector>

namespace VisionGal::Editor
{
	/// Phase 9：列表行 ViewModel 的容量与复用策略入口（slab 化前的集中预留点）。
	/// 当前实现为「单次 Rebuild 前 `clear` + `reserve`」，避免 10k+ 条目下反复 `push_back` 触发多次分配。
	class SequenceEntryStoragePool
	{
	public:
		void PrepareRowVector(std::vector<SequenceEntryViewModel>& rows, unsigned entryCount) const
		{
			rows.clear();
			rows.reserve(entryCount);
		}
	};
}
