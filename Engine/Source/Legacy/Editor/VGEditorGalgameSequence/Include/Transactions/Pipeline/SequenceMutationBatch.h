/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#pragma once

#include "DirtyRegions/SequenceDirtyRegion.h"

#include <memory>
#include <vector>

namespace VisionGal::Editor
{
	class ISequenceEditorCommand;

	/// Phase 9：撤销栈的提交单元。可包含多条 `ISequenceEditorCommand`（内部压栈为
	/// `CompoundSequenceCommand`），`MergedDirtyRegion` 供后续与派生管线对齐扩展使用。
	struct SequenceMutationBatch
	{
		std::vector<std::unique_ptr<ISequenceEditorCommand>> Commands;
		SequenceDirtyRegion MergedDirtyRegion{};
	};
}
