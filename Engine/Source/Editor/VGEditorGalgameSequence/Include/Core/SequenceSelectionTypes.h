/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include <cstdint>

namespace VisionGal::Editor
{
	/// Phase 8：统一 Authoring 选择种类（多面板协同基础）。
	enum class SequenceSelectionKind : uint8_t
	{
		Entry = 0,
		GraphNode,
		GraphEdge,
		TimelineClip,
		RuntimeFrame,
		Group,
	};

	/// 轻量句柄；`ObjectID` 语义依 `Kind`（Entry 时为条目索引编码为 uint64_t）。
	struct SequenceSelectionHandle
	{
		SequenceSelectionKind Kind = SequenceSelectionKind::Entry;
		uint64_t ObjectID = 0;
	};

	inline SequenceSelectionHandle MakeEntrySelectionHandle(const unsigned entryIndex)
	{
		return SequenceSelectionHandle{ SequenceSelectionKind::Entry, static_cast<uint64_t>(entryIndex) };
	}
}
