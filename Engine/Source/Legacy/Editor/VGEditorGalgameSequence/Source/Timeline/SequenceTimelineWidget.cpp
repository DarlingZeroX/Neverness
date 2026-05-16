/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Timeline/SequenceTimelineWidget.h"

#include "Core/SequenceEditorContext.h"

#include <NNRuntimeImGui/IncludeImGui.h>

namespace VisionGal::Editor
{
	void SequenceTimelineWidget::Render(SequenceEditorContext& ctx)
	{
		ImGui::TextUnformatted(u8"时间轴 (v1)");
		const ImVec2 avail = ImGui::GetContentRegionAvail();
		m_controller.DrawAndHandleInteractions(ctx, avail.x, avail.y);
	}
}
