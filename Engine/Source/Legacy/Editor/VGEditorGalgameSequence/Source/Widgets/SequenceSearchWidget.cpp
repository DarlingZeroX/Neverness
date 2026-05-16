/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Widgets/SequenceSearchWidget.h"

#include "Core/SequenceEditorContext.h"
#include "Events/SequenceEditorEventBus.h"

#include <NNRuntimeImGui/IncludeImGui.h>
#include <NNRuntimeImGui/IncludeImGuiEx.h>

namespace VisionGal::Editor
{
	namespace
	{
		void CheckboxDimension(const char* label, SequenceSearchViewModel& vm, SequenceSearchViewModel::Dimension dim)
		{
			bool on = vm.HasDimension(dim);
			if (ImGui::Checkbox(label, &on))
			{
				uint32_t bits = vm.ActiveDimensions();
				if (on)
					bits |= static_cast<uint32_t>(dim);
				else
					bits &= ~static_cast<uint32_t>(dim);
				vm.SetActiveDimensions(bits);
			}
		}
	}

	void SequenceSearchWidget::Render(SequenceEditorContext& ctx)
	{
		ImGuiEx::InputText(u8"搜索##seqSearchPlaceholder", m_searchViewModel.TextFilter());
		ImGui::SameLine();
		using D = SequenceSearchViewModel::Dimension;
		CheckboxDimension(u8"文本##seqDimText", m_searchViewModel, D::TextMatch);
		ImGui::SameLine();
		CheckboxDimension(u8"仅错误##seqDimVal", m_searchViewModel, D::ValidationErrorsOnly);
		ImGui::SameLine();
		CheckboxDimension(u8"仅运行行##seqDimRt", m_searchViewModel, D::RuntimeLineOnly);

		const uint32_t dim = m_searchViewModel.ActiveDimensions();
		const std::string& tf = m_searchViewModel.TextFilter();
		if (tf != m_prevPublishedFilter || dim != m_prevPublishedDimensions)
		{
			m_prevPublishedFilter = tf;
			m_prevPublishedDimensions = dim;
			if (ctx.eventBus != nullptr)
			{
				SequenceEditorEvent ev;
				ev.Type = SequenceEditorEventType::SearchFilterChanged;
				ctx.eventBus->Publish(ev);
			}
			ctx.RequestPresentationRefresh();
		}
	}
}
