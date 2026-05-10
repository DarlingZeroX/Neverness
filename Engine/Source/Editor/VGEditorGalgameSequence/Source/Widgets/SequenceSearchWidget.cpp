/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Widgets/SequenceSearchWidget.h"

#include "Core/SequenceEditorContext.h"

#include <VGImgui/IncludeImGui.h>
#include <VGImgui/IncludeImGuiEx.h>

namespace VisionGal::Editor
{
	void SequenceSearchWidget::Render(SequenceEditorContext& /*ctx*/)
	{
		ImGuiEx::InputText(u8"搜索##seqSearchPlaceholder", m_filter);
	}
}
