/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Widgets/SequenceStatusBarWidget.h"

#include "Core/SequenceEditorContext.h"
#include "Document/SequenceDocument.h"
#include "Runtime/SequenceRuntimeOverlayState.h"
#include "ViewModels/SequenceDocumentViewModel.h"

#include <NNRuntimeImGui/IncludeImGui.h>

namespace VisionGal::Editor
{
	void SequenceStatusBarWidget::Render(SequenceEditorContext& ctx)
	{
		if (ctx.document == nullptr)
			return;

		const std::string& p = ctx.document->GetAssetPath();
		if (p.empty())
			ImGui::TextUnformatted(u8"(未命名)");
		else
			ImGui::TextUnformatted(p.c_str());
		if (ctx.document->IsDirty())
		{
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(1.f, 0.85f, 0.4f, 1.f), "*");
		}

		if (ctx.documentViewModel != nullptr)
		{
			const size_t n = ctx.documentViewModel->GetValidationIssues().size();
			if (n > 0)
			{
				ImGui::SameLine();
				ImGui::TextColored(ImVec4(1.f, 0.75f, 0.35f, 1.f), u8" 校验: %u", static_cast<unsigned>(n));
			}
		}

		const SequenceRuntimeOverlayState* overlay = ctx.runtimeOverlay;
		if (overlay != nullptr && !overlay->LastError.empty())
		{
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(1.f, 0.35f, 0.35f, 1.f), "Run: %s", overlay->LastError.c_str());
		}
	}
}
