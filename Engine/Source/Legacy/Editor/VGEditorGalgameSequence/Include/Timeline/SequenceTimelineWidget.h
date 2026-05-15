/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "Timeline/SequenceTimelineController.h"

namespace VisionGal::Editor
{
	struct SequenceEditorContext;

	class SequenceTimelineWidget
	{
	public:
		void Render(SequenceEditorContext& ctx);

	private:
		SequenceTimelineController m_controller;
	};
}
