/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "Projection/ProjectionEvents/SequenceProjectionEventBus.h"

namespace VisionGal::Editor
{
	class SequenceEditorEventBus;
	class SequenceSelectionModel;

	/// 将投影事件收敛为对 `SequenceSelectionModel` 的单一写入。
	class SequenceSelectionProjectionController
	{
	public:
		void Bind(
			SequenceProjectionEventBus& projectionBus,
			SequenceSelectionModel& selection,
			SequenceEditorEventBus* editorBus);

	private:
		void OnProjectionEvent(const SequenceProjectionEvent& ev);

		SequenceSelectionModel* m_selection = nullptr;
		SequenceEditorEventBus* m_editorBus = nullptr;
	};
}
