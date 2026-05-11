/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Runtime/SequenceRuntimeSession.h"

#include "Events/SequenceEditorEventBus.h"
#include "Runtime/SequenceExecutionController.h"
#include "Runtime/SequenceRuntimeObserver.h"
#include "Runtime/SequenceRuntimeSnapshot.h"

namespace VisionGal::Editor
{
	void SequenceRuntimeSession::Bind(
		SequenceExecutionController* execution,
		SequenceRuntimeObserver* observer,
		SequenceEditorEventBus* bus)
	{
		m_execution = execution;
		m_observer = observer;
		m_bus = bus;
	}

	bool SequenceRuntimeSession::RequestRunTo(
		const std::string& assetPath,
		unsigned targetIndex,
		SequenceRuntimeSnapshot& outSnapshot)
	{
		if (m_execution == nullptr || m_observer == nullptr)
			return false;

		m_state = SequenceRuntimeSessionState::Running;
		const bool ok = m_execution->ExecuteTo(assetPath, targetIndex, outSnapshot);
		m_observer->NotifyExecuteCompleted(outSnapshot, ok);
		m_state = SequenceRuntimeSessionState::Idle;

		if (m_bus != nullptr)
		{
			SequenceEditorEvent ev;
			ev.Type = SequenceEditorEventType::RuntimeStateChanged;
			ev.Runtime.ControllerOk = ok;
			ev.Runtime.ReachedTarget = outSnapshot.ReachedTarget;
			ev.Runtime.HighlightIndex = outSnapshot.CurrentIndex;
			m_bus->Publish(ev);
		}
		return ok;
	}
}
