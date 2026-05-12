/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "Events/SequenceEditorEvent.h"

#include <cstdint>

namespace VisionGal::Editor
{
	class SequenceEditorEventBus;
	class SequenceRuntimeEventTimeline;

	/// 订阅 `RuntimeDebugStream` 并写入 `SequenceRuntimeEventTimeline`（Phase 8 Runtime Bridge）。
	class SequenceRuntimeBridgeRecorder
	{
	public:
		void Bind(SequenceEditorEventBus* editorBus, SequenceRuntimeEventTimeline* timeline);
		void Unbind();
		~SequenceRuntimeBridgeRecorder() { Unbind(); }
		void OnEditorEvent(const SequenceEditorEvent& ev);

		SequenceEditorEventBus* m_bus = nullptr;
		SequenceRuntimeEventTimeline* m_timeline = nullptr;
		uint32_t m_subscriptionToken = 0;
	};
}
