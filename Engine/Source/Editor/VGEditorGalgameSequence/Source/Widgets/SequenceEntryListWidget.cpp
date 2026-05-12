/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Widgets/SequenceEntryListWidget.h"

#include "Commands/MoveSequenceEntryCommand.h"
#include "Commands/RemoveSequenceEntryCommand.h"
#include "Core/SequenceEditorContext.h"
#include "Core/SequenceSelectionModel.h"
#include "Document/SequenceDocument.h"
#include "ViewModels/SequenceDocumentViewModel.h"
#include "ViewModels/SequenceEntryViewModel.h"

#include <VGImgui/IncludeImGui.h>

#include <vector>

namespace VisionGal::Editor
{
	namespace
	{
		constexpr size_t kVirtualizeThreshold = 48;

		std::string RowHeaderLabel(const SequenceEntryViewModel& row)
		{
			std::string header = row.Icon.empty() ? row.DisplayName : (row.Icon + " " + row.DisplayName);
			if (!row.Subtitle.empty())
				header += u8" — ";
			if (!row.Subtitle.empty())
				header += row.Subtitle;
			return header;
		}

		void PushRowAccentColors(const SequenceEntryViewModel& row)
		{
			if (row.RuntimeHighlight)
				ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.55f, 0.38f, 1.f));
			else if (row.HasValidationError)
				ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.55f, 0.22f, 0.22f, 1.f));
		}

		void PopRowAccentColors(const SequenceEntryViewModel& row)
		{
			if (row.RuntimeHighlight || row.HasValidationError)
				ImGui::PopStyleColor();
		}
	}

	void SequenceEntryListWidget::Render(SequenceEditorContext& ctx)
	{
		if (ctx.document == nullptr || ctx.selection == nullptr || ctx.undo == nullptr)
			return;
		if (ctx.documentViewModel == nullptr)
			return;

		const auto& visible = ctx.documentViewModel->GetVisibleEntries();

		std::vector<unsigned> removeIndices;
		const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen;

		int dragSourceIndex = -1;
		int dragTargetIndex = -1;

		const bool useClipper = visible.size() >= kVirtualizeThreshold;

		const auto drawRow = [&](const SequenceEntryViewModel& row) {
			const unsigned realIndex = row.EntryIndex;
			const int payloadIndex = static_cast<int>(realIndex);
			bool show = true;
			const std::string header = RowHeaderLabel(row);

			ImGui::PushID(payloadIndex);

			ImGui::Button(std::to_string(realIndex).c_str());
			ImGui::SameLine();
			if (ImGui::Button("Exec") && ctx.executeToEntry != nullptr)
				(void)ctx.executeToEntry(ctx.executeToUserData, realIndex);
			ImGui::SameLine();

			const bool heldCtrl = ImGui::GetIO().KeyCtrl;

			if (useClipper)
			{
				PushRowAccentColors(row);
				const bool sel = ctx.selection->IsSelected(realIndex);
				if (ImGui::Selectable(header.c_str(), sel, ImGuiSelectableFlags_AllowOverlap))
				{
					if (heldCtrl)
						ctx.selection->ToggleSelection(realIndex);
					else
						ctx.selection->SelectSingle(realIndex);
				}
				PopRowAccentColors(row);
			}
			else
			{
				PushRowAccentColors(row);
				if (ImGui::CollapsingHeader(header.c_str(), &show, flags))
				{
					if (heldCtrl)
					{
						if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
							ctx.selection->ToggleSelection(realIndex);
					}
					else if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
						ctx.selection->SelectSingle(realIndex);
				}
				else
				{
					if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
					{
						if (heldCtrl)
							ctx.selection->ToggleSelection(realIndex);
						else
							ctx.selection->SelectSingle(realIndex);
					}
				}
				PopRowAccentColors(row);
			}

			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
			{
				std::string payloadInfo = header;
				if (ImGui::IsKeyDown(ImGuiKey_Space))
					payloadInfo = header + " (Space to copy)";

				ImGui::SetDragDropPayload("SEQ_ENTRY_INDEX", &payloadIndex, sizeof(int));
				ImGui::Selectable(payloadInfo.c_str());
				ImGui::EndDragDropSource();
			}

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SEQ_ENTRY_INDEX"))
				{
					const int src = *static_cast<const int*>(payload->Data);
					if (src != payloadIndex)
					{
						dragSourceIndex = src;
						dragTargetIndex = payloadIndex;
					}
				}
				ImGui::EndDragDropTarget();
			}

			if (!show)
				removeIndices.push_back(realIndex);

			ImGui::PopID();
		};

		if (useClipper)
		{
			const float lineH = ImGui::GetFrameHeightWithSpacing() * 2.25f;
			ImGuiListClipper clipper;
			clipper.Begin(static_cast<int>(visible.size()), lineH);
			while (clipper.Step())
			{
				for (int line = clipper.DisplayStart; line < clipper.DisplayEnd; ++line)
					drawRow(visible[static_cast<size_t>(line)]);
			}
		}
		else
		{
			for (const auto& row : visible)
				drawRow(row);
		}

		if (dragSourceIndex != -1 && dragTargetIndex != -1 && dragSourceIndex != dragTargetIndex)
		{
			ctx.ExecuteCommand(std::make_unique<MoveSequenceEntryCommand>(
				static_cast<unsigned>(dragSourceIndex), static_cast<unsigned>(dragTargetIndex)));
		}

		if (!removeIndices.empty())
			ctx.ExecuteCommand(std::make_unique<RemoveSequenceEntryCommand>(std::move(removeIndices)));

		ctx.selection->ClampToSize(ctx.document->GetEntryCount());
	}
}
