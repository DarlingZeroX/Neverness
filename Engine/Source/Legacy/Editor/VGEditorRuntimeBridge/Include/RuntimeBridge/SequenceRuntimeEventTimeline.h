/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "RuntimeBridge/SequenceRuntimeEventFrame.h"

#include <vector>

namespace VisionGal::Editor
{
	/// 环形缓冲时间线（仅数据；UI 在 Sequence 模块）。
	class SequenceRuntimeEventTimeline
	{
	public:
		explicit SequenceRuntimeEventTimeline(size_t capacity = 4096);

		void Push(SequenceRuntimeEventFrame frame);
		[[nodiscard]] const std::vector<SequenceRuntimeEventFrame>& GetFrames() const { return m_frames; }
		void Clear();

	private:
		std::vector<SequenceRuntimeEventFrame> m_frames;
		size_t m_capacity = 4096;
	};
}
