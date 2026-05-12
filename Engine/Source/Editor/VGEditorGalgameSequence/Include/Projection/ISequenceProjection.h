/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

namespace VisionGal::Editor
{
	struct SequenceDirtyRegion;
	struct SequenceProjectionContext;

	/// Phase 7/9：Document → 只读展示切片（统一投影上下文）。不得读取 Widget 状态。
	class ISequenceProjection
	{
	public:
		virtual ~ISequenceProjection() = default;

		virtual void Rebuild(const SequenceProjectionContext& ctx) = 0;

		virtual void ApplyDirtyRegion(const SequenceDirtyRegion& dirty, const SequenceProjectionContext& ctx) = 0;
	};
}
