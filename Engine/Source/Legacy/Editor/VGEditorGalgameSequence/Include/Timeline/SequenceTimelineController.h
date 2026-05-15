/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "Timeline/SequenceTimelineLayout.h"

#include <cstddef>

namespace VisionGal::Editor
{
	struct SequenceEditorContext;

	/// Selection + drag reorder coordination for the linear timeline (commands only).
	/// 线性时间轴的选择与拖拽重排协调（仅通过命令）。
	class SequenceTimelineController
	{
	public:
		void DrawAndHandleInteractions(SequenceEditorContext& ctx, float regionWidth, float regionHeight);

	private:
		SequenceTimelineLayout m_layout{};
	};
}
