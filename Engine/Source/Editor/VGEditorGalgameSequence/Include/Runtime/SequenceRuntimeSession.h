/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include <string>

namespace VisionGal::Editor
{
	class SequenceExecutionController;
	class SequenceRuntimeObserver;
	class SequenceEditorEventBus;
	struct SequenceRuntimeSnapshot;

	enum class SequenceRuntimeSessionState : uint8_t
	{
		Idle,
		Running,
		Paused,
		BreakpointHit,
	};

	/// Thin session over `SequenceExecutionController` + overlay observer + bus events.
	class SequenceRuntimeSession
	{
	public:
		void Bind(SequenceExecutionController* execution, SequenceRuntimeObserver* observer, SequenceEditorEventBus* bus);

		[[nodiscard]] SequenceRuntimeSessionState GetState() const { return m_state; }

		bool RequestRunTo(const std::string& assetPath, unsigned targetIndex, SequenceRuntimeSnapshot& outSnapshot);

	private:
		SequenceExecutionController* m_execution = nullptr;
		SequenceRuntimeObserver* m_observer = nullptr;
		SequenceEditorEventBus* m_bus = nullptr;
		SequenceRuntimeSessionState m_state = SequenceRuntimeSessionState::Idle;
	};
}
