/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Widgets/SequenceValidationWidget.h"

#include "Core/SequenceEditorContext.h"
#include "Core/SequenceSelectionModel.h"
#include "ViewModels/SequenceDocumentViewModel.h"

#include <VGImgui/IncludeImGui.h>

namespace VisionGal::Editor
{
	void SequenceValidationWidget::Render(SequenceEditorContext& ctx)
	{
		if (ctx.documentViewModel == nullptr || ctx.selection == nullptr)
			return;

		ImGui::TextUnformatted(u8"校验");
		const auto& issues = ctx.documentViewModel->GetValidationIssues();
		if (issues.empty())
		{
			ImGui::TextDisabled(u8"(无问题)");
			return;
		}

		if (ImGui::BeginChild("SeqValidationList", ImVec2(0, 120), true))
		{
			int issueLine = 0;
			for (const auto& issue : issues)
			{
				ImGui::PushID(issueLine++);
				const std::string rowLabel = "#" + std::to_string(issue.EntryIndex) + " — " + issue.Message;
				if (ImGui::Selectable(rowLabel.c_str(), false))
					ctx.selection->SelectSingle(issue.EntryIndex);
				ImGui::PopID();
			}
		}
		ImGui::EndChild();
	}
}
