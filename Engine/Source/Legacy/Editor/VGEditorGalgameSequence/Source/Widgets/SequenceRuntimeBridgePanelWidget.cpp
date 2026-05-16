/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Widgets/SequenceRuntimeBridgePanelWidget.h"

#include "RuntimeBridge/SequenceRuntimeEventFrame.h"
#include "RuntimeBridge/SequenceRuntimeEventTimeline.h"

#include <NNRuntimeImGui/IncludeImGui.h>

#include <string>

namespace VisionGal::Editor
{
	namespace
	{
		const char* KindLabel(const SequenceRuntimeEventKind k)
		{
			switch (k)
			{
			case SequenceRuntimeEventKind::Step:
				return "Step";
			case SequenceRuntimeEventKind::BreakpointHit:
				return "Breakpoint";
			case SequenceRuntimeEventKind::Pause:
				return "Pause";
			case SequenceRuntimeEventKind::Continue_:
				return "Continue";
			case SequenceRuntimeEventKind::Custom:
				return "Custom";
			default:
				return "?";
			}
		}
	}

	void SequenceRuntimeBridgePanelWidget::Render(SequenceEditorContext& ctx)
	{
		if (ctx.runtimeEventTimeline == nullptr)
		{
			ImGui::TextUnformatted(u8"（无运行时事件时间线）");
			return;
		}
		const auto& frames = ctx.runtimeEventTimeline->GetFrames();
		ImGui::Text(u8"事件帧数: %u", static_cast<unsigned>(frames.size()));
		ImGui::Separator();
		const int n = static_cast<int>(frames.size());
		if (n <= 0)
			return;
		ImGuiListClipper clipper;
		clipper.Begin(n);
		while (clipper.Step())
		{
			for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
			{
				const int ri = n - 1 - i;
				const SequenceRuntimeEventFrame& f = frames[static_cast<size_t>(ri)];
				ImGui::PushID(i);
				ImGui::Text(
					"#%llu [%s] entry=%u %s",
					static_cast<unsigned long long>(f.sequenceNumber),
					KindLabel(f.kind),
					f.entryIndex,
					f.payload.c_str());
				ImGui::PopID();
			}
		}
	}
}
