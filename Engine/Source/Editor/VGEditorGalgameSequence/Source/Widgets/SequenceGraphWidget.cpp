/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Widgets/SequenceGraphWidget.h"
#include "Projection/SequenceGraphProjection.h"
#include "Core/SequenceSelectionModel.h"

#include <VGImgui/IncludeImGui.h>

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

		ImGui::TextUnformatted(u8"Authoring Graph（线性流 + 选择同步）");
		ImGui::Separator();

		for (const auto& node : nodes)
		{
			ImGui::PushID(static_cast<int>(node.EntryIndex));
			const bool sel = ctx.selection->IsSelected(node.EntryIndex);
			const std::string rowLabel = node.Title + " #" + std::to_string(node.EntryIndex);
			if (ImGui::Selectable(
					rowLabel.c_str(),
					sel,
					0,
					ImVec2(0, 0)))
				ctx.selection->SelectSingle(node.EntryIndex);
			if (!node.Subtitle.empty())
				ImGui::TextDisabled("%s", node.Subtitle.c_str());
			ImGui::PopID();
		}
	}
}
