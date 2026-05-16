/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "RuntimeBridge/SequenceRuntimeEventTimeline.h"

namespace VisionGal::Editor
{
	SequenceRuntimeEventTimeline::SequenceRuntimeEventTimeline(const size_t capacity)
		: m_capacity(capacity == 0 ? 4096 : capacity)
	{
		m_frames.reserve(m_capacity);
	}

	void SequenceRuntimeEventTimeline::Push(SequenceRuntimeEventFrame frame)
	{
		const uint64_t nextSeq = m_frames.empty() ? 1u : (m_frames.back().sequenceNumber + 1u);
		frame.sequenceNumber = nextSeq;
		if (m_frames.size() >= m_capacity)
			m_frames.erase(m_frames.begin());
		m_frames.push_back(std::move(frame));
	}

	void SequenceRuntimeEventTimeline::Clear()
	{
		m_frames.clear();
	}
}
