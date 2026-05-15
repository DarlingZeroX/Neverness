/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Events/SequenceEditorEventBus.h"

namespace VisionGal::Editor
{
	uint32_t SequenceEditorEventBus::Subscribe(SequenceEditorEventType type, Listener listener)
	{
		const uint32_t token = m_nextToken++;
		Subscription sub;
		sub.Type = type;
		sub.Callback = std::move(listener);
		m_subscriptions.emplace(token, std::move(sub));
		return token;
	}

	void SequenceEditorEventBus::Unsubscribe(uint32_t token)
	{
		m_subscriptions.erase(token);
	}

	void SequenceEditorEventBus::Publish(const SequenceEditorEvent& event)
	{
		// Copy callbacks so nested Publish does not invalidate iterators.
		std::vector<Listener> toCall;
		toCall.reserve(m_subscriptions.size());
		for (const auto& kv : m_subscriptions)
		{
			if (kv.second.Type == event.Type)
				toCall.push_back(kv.second.Callback);
		}
		for (auto& fn : toCall)
		{
			if (fn)
				fn(event);
		}
	}

	void SequenceEditorEventBus::Clear()
	{
		m_subscriptions.clear();
	}
}
