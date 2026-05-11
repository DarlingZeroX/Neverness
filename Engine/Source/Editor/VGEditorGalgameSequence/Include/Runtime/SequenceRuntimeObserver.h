/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "Runtime/SequenceRuntimeOverlayState.h"

namespace VisionGal::Editor
{
	struct SequenceRuntimeSnapshot;

	/// Pushes snapshot into overlay state for ViewModels / Widgets (no polling in Widgets).
	/// 将快照推入叠加状态供 ViewModel / Widget 使用（Widget 不轮询引擎）。
	class SequenceRuntimeObserver
	{
	public:
		void NotifyExecuteCompleted(const SequenceRuntimeSnapshot& snapshot, bool controllerReturnedOk);

		const SequenceRuntimeOverlayState& GetOverlay() const { return m_overlay; }

	private:
		SequenceRuntimeOverlayState m_overlay{};
	};
}
