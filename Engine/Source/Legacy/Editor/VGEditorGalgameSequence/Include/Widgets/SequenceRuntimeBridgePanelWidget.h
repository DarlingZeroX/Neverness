/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "Core/SequenceEditorContext.h"

namespace VisionGal::Editor
{
	/// 展示 `SequenceRuntimeEventTimeline` 的轻量面板（Phase 8 Runtime Bridge UI）。
	class SequenceRuntimeBridgePanelWidget
	{
	public:
		static void Render(SequenceEditorContext& ctx);
	};
}
