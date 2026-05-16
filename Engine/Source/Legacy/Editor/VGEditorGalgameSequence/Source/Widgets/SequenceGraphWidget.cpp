/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Widgets/SequenceGraphWidget.h"

#include "AuthoringGraph/SequenceAuthoringGraph.h"
#include "Core/SequenceSelectionTypes.h"
#include "Projection/ProjectionEvents/SequenceProjectionEvent.h"
#include "Projection/ProjectionEvents/SequenceProjectionEventBus.h"
#include "Projection/SequenceGraphProjection.h"
#include "Core/SequenceSelectionModel.h"

#include <NNRuntimeImGui/IncludeImGui.h>

namespace VisionGal::Editor
{
	void SequenceGraphWidget::Render(SequenceEditorContext& ctx)
	{
		if (ctx.graphProjection == nullptr || ctx.selection == nullptr)
		{
			ImGui::TextUnformatted(u8"（Graph 投影不可用）");
			return;
		}

		const auto& nodes = ctx.graphProjection->GetNodes();
		if (nodes.empty())
		{
			ImGui::TextUnformatted(u8"（无条目）");
			return;
		}

		ImGui::TextUnformatted(u8"Authoring Graph（线性流 + 投影事件 + 布局）");
		ImGui::Separator();

		ImGuiListClipper clipper;
		clipper.Begin(static_cast<int>(nodes.size()));
		while (clipper.Step())
		{
			for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row)
			{
				const auto& node = nodes[static_cast<size_t>(row)];
				ImGui::PushID(static_cast<int>(node.EntryIndex));
				const bool sel = ctx.selection->IsSelected(node.EntryIndex);
				std::string rowLabel = node.Title + " #" + std::to_string(node.EntryIndex);
				rowLabel += " @(" + std::to_string(static_cast<int>(node.LayoutX)) + "," + std::to_string(static_cast<int>(node.LayoutY)) + ")";
				if (ImGui::Selectable(
						rowLabel.c_str(),
						sel,
						0,
						ImVec2(0, 0)))
				{
					if (ctx.projectionEventBus != nullptr)
					{
						SequenceProjectionSelectionChangedEvent ev;
						ev.Primary = MakeEntrySelectionHandle(node.EntryIndex);
						ctx.projectionEventBus->Publish(ev);
					}
					else
						ctx.selection->SelectSingle(node.EntryIndex);
				}
				if (!node.Subtitle.empty())
					ImGui::TextDisabled("%s", node.Subtitle.c_str());
				if (ctx.authoringGraph != nullptr && ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
				{
					const ImVec2 d = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
					if (d.x != 0.f || d.y != 0.f)
					{
						ctx.authoringGraph->SetNodePositionForEntry(
							node.EntryIndex,
							node.LayoutX + d.x,
							node.LayoutY + d.y);
						ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
						if (ctx.requestPresentationRefresh != nullptr)
							ctx.requestPresentationRefresh(ctx.requestPresentationRefreshUserData);
					}
				}
				ImGui::PopID();
			}
		}
	}
}
