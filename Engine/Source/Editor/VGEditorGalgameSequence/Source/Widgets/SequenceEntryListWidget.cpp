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

#include "VGGalgameScriptSequence/Interface/IVGSSequenceComponent.h"

#include <VGImgui/IncludeImGui.h>

#include <vector>

namespace VisionGal::Editor
{
	void SequenceEntryListWidget::Render(SequenceEditorContext& ctx)
	{
		if (ctx.document == nullptr || ctx.selection == nullptr || ctx.undo == nullptr)
			return;

		unsigned int index = 0;
		std::vector<unsigned> removeIndices;
		const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen;

		int dragSourceIndex = -1;
		int dragTargetIndex = -1;

		const auto seq = ctx.document->GetSequence();
		for (auto& entry : seq->m_Sequence)
		{
			std::string header = entry->GetTypeNameID() + std::to_string(entry->SequenceIndex);
			bool show = true;

			const bool filterActive = ctx.searchFilter != nullptr && !ctx.searchFilter->empty();
			bool pushedDim = false;
			if (filterActive)
			{
				const std::string& f = *ctx.searchFilter;
				if (header.find(f) == std::string::npos && entry->GetTypeNameID().find(f) == std::string::npos)
				{
					ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.35f);
					pushedDim = true;
				}
			}

			ImGui::PushID(static_cast<int>(index));

			ImGui::Button(std::to_string(index).c_str());
			ImGui::SameLine();
			if (ImGui::Button("Exec") && ctx.executeToEntry != nullptr)
				(void)ctx.executeToEntry(ctx.executeToUserData, index);
			ImGui::SameLine();

			const bool heldCtrl = ImGui::GetIO().KeyCtrl;
			if (ImGui::CollapsingHeader(header.c_str(), &show, flags))
			{
				if (heldCtrl)
				{
					if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
						ctx.selection->ToggleSelection(static_cast<uint32_t>(index));
				}
				else if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
					ctx.selection->SelectSingle(static_cast<uint32_t>(index));
			}
			else
			{
				if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
				{
					if (heldCtrl)
						ctx.selection->ToggleSelection(static_cast<uint32_t>(index));
					else
						ctx.selection->SelectSingle(static_cast<uint32_t>(index));
				}
			}

			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
			{
				std::string payloadInfo = header;
				if (ImGui::IsKeyDown(ImGuiKey_Space))
					payloadInfo = header + " (Space to copy)";

				ImGui::SetDragDropPayload("SEQ_ENTRY_INDEX", &index, sizeof(int));
				ImGui::Selectable(payloadInfo.c_str());
				ImGui::EndDragDropSource();
			}

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SEQ_ENTRY_INDEX"))
				{
					const int payloadIndex = *(const int*)payload->Data;
					if (payloadIndex != static_cast<int>(index))
					{
						dragSourceIndex = payloadIndex;
						dragTargetIndex = static_cast<int>(index);
					}
				}
				ImGui::EndDragDropTarget();
			}

			if (!show)
				removeIndices.push_back(index);

			ImGui::PopID();
			index++;

			if (pushedDim)
				ImGui::PopStyleVar();
		}

		if (dragSourceIndex != -1 && dragTargetIndex != -1 && dragSourceIndex != dragTargetIndex)
		{
			ctx.ExecuteCommand(std::make_unique<MoveSequenceEntryCommand>(
				static_cast<unsigned>(dragSourceIndex), static_cast<unsigned>(dragTargetIndex)));
		}

		if (!removeIndices.empty())
			ctx.ExecuteCommand(std::make_unique<RemoveSequenceEntryCommand>(std::move(removeIndices)));

		ctx.selection->ClampToSize(seq->m_Sequence.size());
	}
}
