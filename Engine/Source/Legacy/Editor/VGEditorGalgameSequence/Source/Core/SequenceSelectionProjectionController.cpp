/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Core/SequenceSelectionProjectionController.h"

#include "Core/SequenceSelectionModel.h"
#include "Events/SequenceEditorEventBus.h"
#include "Projection/ProjectionEvents/SequenceProjectionEvent.h"

namespace VisionGal::Editor
{
	void SequenceSelectionProjectionController::Bind(
		SequenceProjectionEventBus& projectionBus,
		SequenceSelectionModel& selection,
		SequenceEditorEventBus* editorBus)
	{
		m_selection = &selection;
		m_editorBus = editorBus;
		projectionBus.Subscribe([this](const SequenceProjectionEvent& ev) { OnProjectionEvent(ev); });
	}

	void SequenceSelectionProjectionController::OnProjectionEvent(const SequenceProjectionEvent& ev)
	{
		if (m_selection == nullptr)
			return;
		if (const auto* sel = std::get_if<SequenceProjectionSelectionChangedEvent>(&ev))
		{
			if (sel->Primary.Kind == SequenceSelectionKind::Entry)
				m_selection->SelectSingle(static_cast<uint32_t>(sel->Primary.ObjectID));
			return;
		}
		(void)m_editorBus;
	}
}
