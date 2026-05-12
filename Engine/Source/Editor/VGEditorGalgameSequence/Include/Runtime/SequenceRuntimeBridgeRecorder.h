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

	/// Phase 8：曾订阅 `RuntimeDebugStream` 写入时间线。
	/// Phase 9 起时间线由 **`SequenceRuntimeKernel::EmitDebugStream`** 直接写入；本类保留以便外部工具链或
	/// 旧代码编译兼容，宿主默认不再调用 **`Bind`**。
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
