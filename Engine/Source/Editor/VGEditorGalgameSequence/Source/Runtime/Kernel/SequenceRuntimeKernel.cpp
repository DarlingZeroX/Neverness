/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Runtime/Kernel/SequenceRuntimeKernel.h"

#include "Events/SequenceEditorEventBus.h"
#include "Runtime/SequenceExecutionController.h"
#include "Runtime/SequenceRuntimeObserver.h"
#include "Runtime/SequenceRuntimeSnapshot.h"
#include "RuntimeBridge/SequenceRuntimeEventFrame.h"
#include "RuntimeBridge/SequenceRuntimeEventTimeline.h"

#include "Runtime/SequenceRuntimePropertySnapshot.h"

#include <optional>

namespace VisionGal::Editor
{
	namespace
	{
		SequenceRuntimeEventKind MapTimelineKind(const SequenceRuntimeStreamEventKind k)
		{
			switch (k)
			{
			case SequenceRuntimeStreamEventKind::RuntimeStepped:
				return SequenceRuntimeEventKind::Step;
			case SequenceRuntimeStreamEventKind::RuntimeBreakpointHit:
				return SequenceRuntimeEventKind::BreakpointHit;
			case SequenceRuntimeStreamEventKind::RuntimePaused:
				return SequenceRuntimeEventKind::Pause;
			case SequenceRuntimeStreamEventKind::RuntimeResumed:
				return SequenceRuntimeEventKind::Continue_;
			case SequenceRuntimeStreamEventKind::RuntimeNodeEntered:
				return SequenceRuntimeEventKind::Custom;
			case SequenceRuntimeStreamEventKind::RuntimePropertyChanged:
				return SequenceRuntimeEventKind::Custom;
			default:
				return SequenceRuntimeEventKind::Unknown;
			}
		}
	}

	void SequenceRuntimeKernel::Bind(
		SequenceExecutionController* execution,
		SequenceRuntimeObserver* observer,
		SequenceEditorEventBus* bus,
		SequenceRuntimeEventTimeline* timeline)
	{
		m_execution = execution;
		m_observer = observer;
		m_bus = bus;
		m_timeline = timeline;
	}

	void SequenceRuntimeKernel::EmitDebugStream(
		const SequenceRuntimeStreamEventKind kind,
		const unsigned index,
		const std::string& message,
		const bool ok,
		const bool reached)
	{
		if (m_timeline != nullptr)
		{
			SequenceRuntimeEventFrame frame;
			frame.kind = MapTimelineKind(kind);
			frame.entryIndex = index;
			frame.payload = message;
			m_timeline->Push(std::move(frame));
		}
		if (m_bus != nullptr)
		{
			SequenceEditorEvent ev;
			ev.Type = SequenceEditorEventType::RuntimeDebugStream;
			ev.RuntimeStream.Kind = kind;
			ev.RuntimeStream.Index = index;
			ev.RuntimeStream.Message = message;
			ev.RuntimeStream.ControllerOk = ok;
			ev.RuntimeStream.ReachedTarget = reached;
			ev.RuntimeStream.PropertyWatch = std::nullopt;
			m_bus->Publish(ev);
		}
	}

	bool SequenceRuntimeKernel::ExecuteTo(
		const std::string& assetPath,
		const unsigned targetEntryIndex,
		SequenceRuntimeSnapshot& out)
	{
		if (m_execution == nullptr)
			return false;
		return m_execution->ExecuteTo(assetPath, targetEntryIndex, out);
	}

	void SequenceRuntimeKernel::EmitPropertyWatch(const SequenceRuntimePropertySnapshot& snap)
	{
		if (m_timeline != nullptr)
		{
			SequenceRuntimeEventFrame frame;
			frame.kind = SequenceRuntimeEventKind::Custom;
			frame.entryIndex = snap.EntryIndex;
			frame.payload = snap.PropertyPath + ": " + snap.OldValueText + " -> " + snap.NewValueText;
			frame.timestampSeconds = snap.TimestampSeconds;
			m_timeline->Push(std::move(frame));
		}
		if (m_bus != nullptr)
		{
			SequenceEditorEvent ev;
			ev.Type = SequenceEditorEventType::RuntimeDebugStream;
			ev.RuntimeStream.Kind = SequenceRuntimeStreamEventKind::RuntimePropertyChanged;
			ev.RuntimeStream.Index = snap.EntryIndex;
			ev.RuntimeStream.Message = snap.PropertyPath;
			ev.RuntimeStream.ControllerOk = true;
			ev.RuntimeStream.ReachedTarget = false;
			ev.RuntimeStream.PropertyWatch = snap;
			m_bus->Publish(ev);
		}
	}
}

