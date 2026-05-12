/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Runtime/SequenceDebuggerSession.h"

#include "Events/SequenceEditorEvent.h"
#include "Events/SequenceEditorEventBus.h"
#include "Runtime/SequenceExecutionController.h"
#include "Runtime/SequenceRuntimeObserver.h"
#include "Runtime/SequenceRuntimeSnapshot.h"

#include <algorithm>
#include <limits>

namespace VisionGal::Editor
{
	void SequenceDebuggerSession::Bind(
		SequenceExecutionController* execution,
		SequenceRuntimeObserver* observer,
		SequenceEditorEventBus* bus)
	{
		m_execution = execution;
		m_observer = observer;
		m_bus = bus;
		PushBreakpointsToOverlay();
	}

	void SequenceDebuggerSession::PublishStream(
		const SequenceRuntimeStreamEventKind kind,
		const unsigned index,
		const std::string& message,
		const bool ok,
		const bool reached)
	{
		if (m_bus == nullptr)
			return;
		SequenceEditorEvent ev;
		ev.Type = SequenceEditorEventType::RuntimeDebugStream;
		ev.RuntimeStream.Kind = kind;
		ev.RuntimeStream.Index = index;
		ev.RuntimeStream.Message = message;
		ev.RuntimeStream.ControllerOk = ok;
		ev.RuntimeStream.ReachedTarget = reached;
		m_bus->Publish(ev);
	}

	void SequenceDebuggerSession::SortBreakpointsUnique()
	{
		std::sort(m_breakpoints.begin(), m_breakpoints.end());
		m_breakpoints.erase(std::unique(m_breakpoints.begin(), m_breakpoints.end()), m_breakpoints.end());
	}

	void SequenceDebuggerSession::PushBreakpointsToOverlay()
	{
		if (m_observer == nullptr)
			return;
		std::vector<uint32_t> v;
		v.reserve(m_breakpoints.size());
		for (const unsigned x : m_breakpoints)
			v.push_back(static_cast<uint32_t>(x));
		m_observer->SetBreakpointMarkers(std::move(v));
	}

	void SequenceDebuggerSession::Pause()
	{
		m_pauseRequested = true;
		PublishStream(SequenceRuntimeStreamEventKind::RuntimePaused, 0, {}, true, false);
	}

	void SequenceDebuggerSession::Resume()
	{
		m_pauseRequested = false;
		PublishStream(SequenceRuntimeStreamEventKind::RuntimeResumed, 0, {}, true, false);
	}

	void SequenceDebuggerSession::ToggleBreakpoint(const unsigned entryIndex)
	{
		const auto it = std::lower_bound(m_breakpoints.begin(), m_breakpoints.end(), entryIndex);
		if (it != m_breakpoints.end() && *it == entryIndex)
			m_breakpoints.erase(it);
		else
			m_breakpoints.insert(it, entryIndex);
		SortBreakpointsUnique();
		PushBreakpointsToOverlay();
	}

	void SequenceDebuggerSession::ClearBreakpoints()
	{
		m_breakpoints.clear();
		PushBreakpointsToOverlay();
	}

	bool SequenceDebuggerSession::RequestRunTo(
		const std::string& assetPath,
		const unsigned targetIndex,
		SequenceRuntimeSnapshot& outSnapshot)
	{
		if (m_execution == nullptr || m_observer == nullptr)
			return false;

		m_state = SequenceDebuggerSessionState::Running;
		PublishStream(SequenceRuntimeStreamEventKind::RuntimeStarted, targetIndex, {}, true, false);

		const bool ok = m_execution->ExecuteTo(assetPath, targetIndex, outSnapshot);
		m_observer->NotifyExecuteCompleted(outSnapshot, ok);

		if (m_bus != nullptr)
		{
			SequenceEditorEvent ev;
			ev.Type = SequenceEditorEventType::RuntimeStateChanged;
			ev.Runtime.ControllerOk = ok;
			ev.Runtime.ReachedTarget = outSnapshot.ReachedTarget;
			ev.Runtime.HighlightIndex = outSnapshot.CurrentIndex;
			m_bus->Publish(ev);
		}

		if (ok)
		{
			PublishStream(
				SequenceRuntimeStreamEventKind::RuntimeFinished,
				outSnapshot.CurrentIndex,
				{},
				true,
				outSnapshot.ReachedTarget);
		}
		else
		{
			PublishStream(
				SequenceRuntimeStreamEventKind::RuntimeError,
				outSnapshot.CurrentIndex,
				outSnapshot.LastError,
				false,
				false);
		}

		m_state = SequenceDebuggerSessionState::Idle;
		return ok;
	}

	bool SequenceDebuggerSession::Step(const std::string& assetPath, SequenceRuntimeSnapshot& outSnapshot)
	{
		if (m_execution == nullptr || m_observer == nullptr)
			return false;

		std::string err;
		if (!m_execution->BeginDebugSession(assetPath, err))
		{
			outSnapshot = SequenceRuntimeSnapshot{};
			outSnapshot.LastError = err;
			PublishStream(SequenceRuntimeStreamEventKind::RuntimeError, 0, err, false, false);
			return false;
		}

		m_state = SequenceDebuggerSessionState::Running;
		PublishStream(SequenceRuntimeStreamEventKind::RuntimeStarted, 0, {}, true, false);

		const bool ok = m_execution->StepOnce(outSnapshot, err);
		if (!ok)
			outSnapshot.LastError = err;

		m_observer->NotifyExecuteCompleted(outSnapshot, ok);
		if (m_bus != nullptr)
		{
			SequenceEditorEvent ev;
			ev.Type = SequenceEditorEventType::RuntimeStateChanged;
			ev.Runtime.ControllerOk = ok;
			ev.Runtime.ReachedTarget = false;
			ev.Runtime.HighlightIndex = outSnapshot.CurrentIndex;
			m_bus->Publish(ev);
		}

		PublishStream(SequenceRuntimeStreamEventKind::RuntimeNodeEntered, outSnapshot.CurrentIndex, {}, ok, false);
		PublishStream(SequenceRuntimeStreamEventKind::RuntimeStepped, outSnapshot.CurrentIndex, err, ok, false);

		m_state = ok ? SequenceDebuggerSessionState::Paused : SequenceDebuggerSessionState::Error;
		return ok;
	}

	bool SequenceDebuggerSession::ContinueToSelectionOrBreakpoint(
		const std::string& assetPath,
		const unsigned targetIndex,
		SequenceRuntimeSnapshot& outSnapshot)
	{
		if (m_execution == nullptr || m_observer == nullptr)
			return false;

		std::string err;
		if (!m_execution->BeginDebugSession(assetPath, err))
		{
			outSnapshot = SequenceRuntimeSnapshot{};
			outSnapshot.LastError = err;
			return false;
		}

		m_state = SequenceDebuggerSessionState::Running;
		m_pauseRequested = false;
		PublishStream(SequenceRuntimeStreamEventKind::RuntimeStarted, targetIndex, {}, true, false);

		const unsigned kOpen = std::numeric_limits<unsigned>::max();
		const unsigned effectiveTarget = (targetIndex == kOpen) ? kOpen : targetIndex;
		std::vector<unsigned> sortedBp = m_breakpoints;
		std::sort(sortedBp.begin(), sortedBp.end());
		sortedBp.erase(std::unique(sortedBp.begin(), sortedBp.end()), sortedBp.end());
		const std::vector<unsigned>* bpPtr = sortedBp.empty() ? nullptr : &sortedBp;

		const bool ok = m_execution->ContinueExecution(
			effectiveTarget,
			bpPtr,
			true,
			&m_pauseRequested,
			outSnapshot,
			err);
		if (!ok)
			outSnapshot.LastError = err;

		m_observer->NotifyExecuteCompleted(outSnapshot, ok);
		if (m_bus != nullptr)
		{
			SequenceEditorEvent ev;
			ev.Type = SequenceEditorEventType::RuntimeStateChanged;
			ev.Runtime.ControllerOk = ok;
			ev.Runtime.ReachedTarget = outSnapshot.ReachedTarget;
			ev.Runtime.HighlightIndex = outSnapshot.CurrentIndex;
			m_bus->Publish(ev);
		}

		if (outSnapshot.BreakpointHit)
			PublishStream(
				SequenceRuntimeStreamEventKind::RuntimeBreakpointHit,
				outSnapshot.CurrentIndex,
				{},
				ok,
				false);
		else if (outSnapshot.StoppedOnPause)
			PublishStream(SequenceRuntimeStreamEventKind::RuntimePaused, outSnapshot.CurrentIndex, {}, ok, false);
		else if (ok && outSnapshot.ReachedTarget)
			PublishStream(
				SequenceRuntimeStreamEventKind::RuntimeFinished,
				outSnapshot.CurrentIndex,
				{},
				true,
				true);
		else if (!ok)
			PublishStream(SequenceRuntimeStreamEventKind::RuntimeError, outSnapshot.CurrentIndex, err, false, false);
		else
			PublishStream(SequenceRuntimeStreamEventKind::RuntimeFinished, outSnapshot.CurrentIndex, {}, ok, false);

		m_state = SequenceDebuggerSessionState::Idle;
		return ok;
	}
}
