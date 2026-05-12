/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Runtime/SequenceRuntimeBridgeRecorder.h"

#include "Events/SequenceEditorEventBus.h"
#include "RuntimeBridge/SequenceRuntimeEventFrame.h"
#include "RuntimeBridge/SequenceRuntimeEventTimeline.h"

namespace VisionGal::Editor
{
	namespace
	{
		SequenceRuntimeEventKind MapKind(const SequenceRuntimeStreamEventKind k)
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
			default:
				return SequenceRuntimeEventKind::Unknown;
			}
		}
	}

	void SequenceRuntimeBridgeRecorder::Bind(SequenceEditorEventBus* editorBus, SequenceRuntimeEventTimeline* timeline)
	{
		Unbind();
		m_bus = editorBus;
		m_timeline = timeline;
		if (m_bus == nullptr || m_timeline == nullptr)
			return;
		m_subscriptionToken = m_bus->Subscribe(
			SequenceEditorEventType::RuntimeDebugStream,
			[this](const SequenceEditorEvent& ev) { OnEditorEvent(ev); });
	}

	void SequenceRuntimeBridgeRecorder::Unbind()
	{
		if (m_bus != nullptr && m_subscriptionToken != 0)
			m_bus->Unsubscribe(m_subscriptionToken);
		m_subscriptionToken = 0;
		m_bus = nullptr;
		m_timeline = nullptr;
	}

	void SequenceRuntimeBridgeRecorder::OnEditorEvent(const SequenceEditorEvent& ev)
	{
		if (m_timeline == nullptr || ev.Type != SequenceEditorEventType::RuntimeDebugStream)
			return;
		SequenceRuntimeEventFrame frame;
		frame.kind = MapKind(ev.RuntimeStream.Kind);
		frame.entryIndex = ev.RuntimeStream.Index;
		frame.payload = ev.RuntimeStream.Message;
		m_timeline->Push(std::move(frame));
	}
}
