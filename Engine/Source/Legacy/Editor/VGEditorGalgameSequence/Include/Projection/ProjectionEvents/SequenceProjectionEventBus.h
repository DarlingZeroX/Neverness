/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "Projection/ProjectionEvents/SequenceProjectionEvent.h"

#include <functional>
#include <vector>

namespace VisionGal::Editor
{
	/// 投影层专用同步总线（同线程），与 `SequenceEditorEventBus` 解耦。
	class SequenceProjectionEventBus
	{
	public:
		using Handler = std::function<void(const SequenceProjectionEvent&)>;
		void Subscribe(Handler handler) { m_handlers.push_back(std::move(handler)); }
		void Publish(const SequenceProjectionEvent& event) const
		{
			for (const Handler& h : m_handlers)
				h(event);
		}

	private:
		std::vector<Handler> m_handlers;
	};
}
