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
#include <string>

namespace VisionGal::Editor
{
	class SequenceEditorEventBus;
	class SequenceExecutionController;
	class SequenceRuntimeEventTimeline;
	class SequenceRuntimeObserver;
	struct SequenceRuntimeSnapshot;

	/// Phase 9：单核运行时入口。封装 `SequenceExecutionController` + `SequenceRuntimeObserver`，并作为
	/// **调试流与时间线的唯一写入源**（`EmitDebugStream`：先写 `SequenceRuntimeEventTimeline`，再可选转发总线）。
	class SequenceRuntimeKernel
	{
	public:
		void Bind(
			SequenceExecutionController* execution,
			SequenceRuntimeObserver* observer,
			SequenceEditorEventBus* bus,
			SequenceRuntimeEventTimeline* timeline);

		[[nodiscard]] SequenceExecutionController* GetExecutionController() const { return m_execution; }
		[[nodiscard]] SequenceRuntimeObserver* GetRuntimeObserver() const { return m_observer; }
		[[nodiscard]] SequenceEditorEventBus* GetEventBus() const { return m_bus; }

		void EmitDebugStream(
			SequenceRuntimeStreamEventKind kind,
			unsigned index,
			const std::string& message,
			bool ok,
			bool reached);

		[[nodiscard]] bool ExecuteTo(
			const std::string& assetPath,
			unsigned targetEntryIndex,
			SequenceRuntimeSnapshot& out);

	private:
		SequenceExecutionController* m_execution = nullptr;
		SequenceRuntimeObserver* m_observer = nullptr;
		SequenceEditorEventBus* m_bus = nullptr;
		SequenceRuntimeEventTimeline* m_timeline = nullptr;
	};
}
