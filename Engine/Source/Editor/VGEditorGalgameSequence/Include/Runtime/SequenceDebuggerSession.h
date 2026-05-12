/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "Events/SequenceEditorEvent.h"

namespace VisionGal::Editor
{
	class SequenceExecutionController;
	class SequenceRuntimeObserver;
	class SequenceEditorEventBus;
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

	/// Editor-side debug session: step / continue / breakpoints + runtime event stream.
	class SequenceDebuggerSession
	{
	public:
		void Bind(SequenceExecutionController* execution, SequenceRuntimeObserver* observer, SequenceEditorEventBus* bus);

		[[nodiscard]] SequenceDebuggerSessionState GetState() const { return m_state; }

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

		SequenceExecutionController* m_execution = nullptr;
		SequenceRuntimeObserver* m_observer = nullptr;
		SequenceEditorEventBus* m_bus = nullptr;
		SequenceDebuggerSessionState m_state = SequenceDebuggerSessionState::Idle;
		std::vector<unsigned> m_breakpoints;
		bool m_pauseRequested = false;
	};
}
