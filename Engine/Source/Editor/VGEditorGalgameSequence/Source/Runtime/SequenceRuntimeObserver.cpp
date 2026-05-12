/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Runtime/SequenceRuntimeObserver.h"

#include "Runtime/SequenceRuntimeSnapshot.h"

namespace VisionGal::Editor
{
	void SequenceRuntimeObserver::SetBreakpointMarkers(std::vector<uint32_t> indices)
	{
		m_overlay.BreakpointIndices = std::move(indices);
	}

	void SequenceRuntimeObserver::NotifyExecuteCompleted(const SequenceRuntimeSnapshot& snapshot, bool controllerReturnedOk)
	{
		m_overlay.LastError = snapshot.LastError;
		m_overlay.ReachedTarget = snapshot.ReachedTarget;
		m_overlay.HighlightIndex = snapshot.CurrentIndex;
		m_overlay.ShowExecutionLine = snapshot.HasValidDebugInfo && controllerReturnedOk;
		if (!snapshot.LastError.empty())
			m_overlay.ShowExecutionLine = snapshot.HasValidDebugInfo;
	}
}
