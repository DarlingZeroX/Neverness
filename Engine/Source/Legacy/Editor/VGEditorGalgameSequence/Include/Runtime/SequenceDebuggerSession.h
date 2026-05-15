/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "Runtime/Kernel/SequenceRuntimeKernel.h"

#include <limits>
#include <string>
#include <vector>

#include "Events/SequenceEditorEvent.h"

namespace VisionGal::Editor
{
	class SequenceExecutionController;
	class SequenceRuntimeObserver;
	class SequenceEditorEventBus;
	class SequenceRuntimeEventTimeline;
	struct SequenceRuntimeSnapshot;

	enum class SequenceDebuggerSessionState : uint8_t
	{
		Idle,
		Running,
		Paused,
		BreakpointHit,
		Finished,
		Error,
	};

	/// Editor-side debug session: step / continue / breakpoints + runtime event stream（经 `SequenceRuntimeKernel` 单核）。
	class SequenceDebuggerSession
	{
	public:
		void Bind(
			SequenceExecutionController* execution,
			SequenceRuntimeObserver* observer,
			SequenceEditorEventBus* bus,
			SequenceRuntimeEventTimeline* runtimeTimeline);

		[[nodiscard]] SequenceDebuggerSessionState GetState() const { return m_state; }

		[[nodiscard]] SequenceRuntimeKernel& GetRuntimeKernel() { return m_kernel; }
		[[nodiscard]] const SequenceRuntimeKernel& GetRuntimeKernel() const { return m_kernel; }

		bool RequestRunTo(const std::string& assetPath, unsigned targetIndex, SequenceRuntimeSnapshot& outSnapshot);

		bool Step(const std::string& assetPath, SequenceRuntimeSnapshot& outSnapshot);
		bool ContinueToSelectionOrBreakpoint(const std::string& assetPath, unsigned targetIndex, SequenceRuntimeSnapshot& outSnapshot);
		void Pause();
		void Resume();
		void ToggleBreakpoint(unsigned entryIndex);
		void ClearBreakpoints();
		[[nodiscard]] const std::vector<unsigned>& GetBreakpoints() const { return m_breakpoints; }

	private:
		void PublishStream(SequenceRuntimeStreamEventKind kind, unsigned index, const std::string& message, bool ok, bool reached);
		void PushBreakpointsToOverlay();
		void SortBreakpointsUnique();

		SequenceRuntimeKernel m_kernel;
		SequenceDebuggerSessionState m_state = SequenceDebuggerSessionState::Idle;
		std::vector<unsigned> m_breakpoints;
		bool m_pauseRequested = false;
		unsigned m_watchLastIndex = (std::numeric_limits<unsigned>::max)();
		std::string m_watchLastComponentType;
	};
}
