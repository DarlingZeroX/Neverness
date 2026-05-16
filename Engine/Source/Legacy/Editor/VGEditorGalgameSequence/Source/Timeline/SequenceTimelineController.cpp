/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Timeline/SequenceTimelineController.h"

#include "Commands/MoveSequenceEntryCommand.h"
#include "Core/SequenceEditorContext.h"
#include "Core/SequenceSelectionModel.h"
#include "Core/SequenceSelectionTypes.h"
#include "Projection/ProjectionEvents/SequenceProjectionEvent.h"
#include "Projection/ProjectionEvents/SequenceProjectionEventBus.h"
#include "ViewModels/SequenceDocumentViewModel.h"
#include "ViewModels/SequenceEntryViewModel.h"

#include <NNRuntimeImGui/IncludeImGui.h>

#include <algorithm>

namespace VisionGal::Editor
{
	namespace
	{
		void PublishOrSelectEntry(SequenceEditorContext& ctx, const unsigned realIndex, const bool toggle)
		{
			if (toggle)
			{
				ctx.selection->ToggleSelection(static_cast<uint32_t>(realIndex));
				return;
			}
			if (ctx.projectionEventBus != nullptr)
			{
				SequenceProjectionSelectionChangedEvent ev;
				ev.Primary = MakeEntrySelectionHandle(realIndex);
				ctx.projectionEventBus->Publish(ev);
			}
			else
				ctx.selection->SelectSingle(static_cast<uint32_t>(realIndex));
		}
	}

	void SequenceTimelineController::DrawAndHandleInteractions(SequenceEditorContext& ctx, float regionWidth, float regionHeight)
	{
		if (ctx.documentViewModel == nullptr || ctx.selection == nullptr || ctx.undo == nullptr)
			return;

		const auto& rows = ctx.documentViewModel->GetVisibleEntries();
		if (rows.empty())
			return;

		int dragSourceIndex = -1;
		int dragTargetIndex = -1;

		const ImGuiWindowFlags childFlags = ImGuiWindowFlags_HorizontalScrollbar;
		const float height = std::max(120.f, std::min(regionHeight, m_layout.RowHeight * static_cast<float>(rows.size() + 2)));
		if (ImGui::BeginChild("SeqTimelineInner", ImVec2(regionWidth, height), true, childFlags))
		{
			const bool useClipper = rows.size() > 64u;
		if (useClipper)
		{
			ImGuiListClipper clipper;
			clipper.Begin(static_cast<int>(rows.size()), m_layout.RowHeight);
			while (clipper.Step())
			{
				for (int v = clipper.DisplayStart; v < clipper.DisplayEnd; ++v)
				{
					const SequenceEntryViewModel& row = rows[static_cast<size_t>(v)];
					const unsigned realIndex = row.EntryIndex;
					ImGui::PushID(static_cast<int>(realIndex));

					const bool selected = ctx.selection->IsSelected(static_cast<uint32_t>(realIndex));
					if (row.RuntimeHighlight)
						ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.25f, 0.45f, 0.35f, 1.f));
					else if (row.HasValidationError)
						ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.5f, 0.22f, 0.22f, 1.f));

					const std::string label = row.Icon.empty() ? row.DisplayName : (row.Icon + " " + row.DisplayName);
					const ImVec2 p0 = ImGui::GetCursorScreenPos();
					const float rowW = ImGui::GetContentRegionAvail().x;
					const ImVec2 p1(p0.x + rowW, p0.y + m_layout.RowHeight);

					if (ImGui::InvisibleButton("row", ImVec2(rowW, m_layout.RowHeight)))
					{
						const bool heldCtrl = ImGui::GetIO().KeyCtrl;
						PublishOrSelectEntry(ctx, realIndex, heldCtrl);
					}

					ImDrawList* dl = ImGui::GetWindowDrawList();
					const ImU32 bg = selected ? IM_COL32(70, 90, 130, 255) : IM_COL32(45, 45, 52, 255);
					dl->AddRectFilled(p0, p1, bg, 2.f);
					dl->AddText(ImVec2(p0.x + 6.f, p0.y + 5.f), IM_COL32(240, 240, 240, 255), label.c_str());

					if (row.RuntimeHighlight || row.HasValidationError)
						ImGui::PopStyleColor();

					if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
					{
						int idx = static_cast<int>(realIndex);
						ImGui::SetDragDropPayload("SEQ_ENTRY_INDEX", &idx, sizeof(int));
						ImGui::TextUnformatted(label.c_str());
						ImGui::EndDragDropSource();
					}

					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SEQ_ENTRY_INDEX"))
						{
							const int payloadIndex = *static_cast<const int*>(payload->Data);
							if (payloadIndex != static_cast<int>(realIndex))
							{
								dragSourceIndex = payloadIndex;
								dragTargetIndex = static_cast<int>(realIndex);
							}
						}
						ImGui::EndDragDropTarget();
					}

					ImGui::SetCursorScreenPos(ImVec2(p0.x, p0.y + m_layout.RowHeight));
					ImGui::PopID();
				}
			}
		}
		else
		{
			for (size_t v = 0; v < rows.size(); ++v)
			{
				const SequenceEntryViewModel& row = rows[v];
				const unsigned realIndex = row.EntryIndex;
				ImGui::PushID(static_cast<int>(realIndex));

				const bool selected = ctx.selection->IsSelected(static_cast<uint32_t>(realIndex));
				if (row.RuntimeHighlight)
					ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.25f, 0.45f, 0.35f, 1.f));
				else if (row.HasValidationError)
					ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.5f, 0.22f, 0.22f, 1.f));

				const std::string label = row.Icon.empty() ? row.DisplayName : (row.Icon + " " + row.DisplayName);
				const ImVec2 p0 = ImGui::GetCursorScreenPos();
				const float rowW = ImGui::GetContentRegionAvail().x;
				const ImVec2 p1(p0.x + rowW, p0.y + m_layout.RowHeight);

				if (ImGui::InvisibleButton("row", ImVec2(rowW, m_layout.RowHeight)))
				{
					const bool heldCtrl = ImGui::GetIO().KeyCtrl;
					PublishOrSelectEntry(ctx, realIndex, heldCtrl);
				}

				ImDrawList* dl = ImGui::GetWindowDrawList();
				const ImU32 bg = selected ? IM_COL32(70, 90, 130, 255) : IM_COL32(45, 45, 52, 255);
				dl->AddRectFilled(p0, p1, bg, 2.f);
				dl->AddText(ImVec2(p0.x + 6.f, p0.y + 5.f), IM_COL32(240, 240, 240, 255), label.c_str());

				if (row.RuntimeHighlight || row.HasValidationError)
					ImGui::PopStyleColor();

				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
				{
					int idx = static_cast<int>(realIndex);
					ImGui::SetDragDropPayload("SEQ_ENTRY_INDEX", &idx, sizeof(int));
					ImGui::TextUnformatted(label.c_str());
					ImGui::EndDragDropSource();
				}

				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SEQ_ENTRY_INDEX"))
					{
						const int payloadIndex = *static_cast<const int*>(payload->Data);
						if (payloadIndex != static_cast<int>(realIndex))
						{
							dragSourceIndex = payloadIndex;
							dragTargetIndex = static_cast<int>(realIndex);
						}
					}
					ImGui::EndDragDropTarget();
				}

				ImGui::SetCursorScreenPos(ImVec2(p0.x, p0.y + m_layout.RowHeight));
				ImGui::PopID();
			}
		}
		}
		ImGui::EndChild();

		if (dragSourceIndex != -1 && dragTargetIndex != -1 && dragSourceIndex != dragTargetIndex)
		{
			ctx.ExecuteCommand(std::make_unique<MoveSequenceEntryCommand>(
				static_cast<unsigned>(dragSourceIndex), static_cast<unsigned>(dragTargetIndex)));
		}
	}
}
