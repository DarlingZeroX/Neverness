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
#include <functional>
#include <unordered_map>
#include <vector>

namespace VisionGal::Editor
{
	/// Single-threaded (ImGui frame) pub/sub for editor panels and tests.
	class SequenceEditorEventBus
	{
	public:
		using Listener = std::function<void(const SequenceEditorEvent&)>;

		[[nodiscard]] uint32_t Subscribe(SequenceEditorEventType type, Listener listener);
		void Unsubscribe(uint32_t token);
		void Publish(const SequenceEditorEvent& event);

		void Clear();

	private:
		struct Subscription
		{
			SequenceEditorEventType Type = SequenceEditorEventType::DocumentChanged;
			Listener Callback{};
		};

		uint32_t m_nextToken = 1;
		std::unordered_map<uint32_t, Subscription> m_subscriptions;
	};
}
