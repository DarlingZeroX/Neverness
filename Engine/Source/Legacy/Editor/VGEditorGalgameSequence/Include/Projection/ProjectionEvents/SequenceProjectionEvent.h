/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "Core/SequenceSelectionTypes.h"

#include <variant>

namespace VisionGal::Editor
{
	struct SequenceProjectionSelectionChangedEvent
	{
		SequenceSelectionHandle Primary{};
	};

	struct SequenceProjectionViewportScrollEvent
	{
		float ScrollX = 0.f;
		float ScrollY = 0.f;
	};

	struct SequenceProjectionNavigateToEntryEvent
	{
		unsigned EntryIndex = 0;
	};

	using SequenceProjectionEvent = std::variant<
		SequenceProjectionSelectionChangedEvent,
		SequenceProjectionViewportScrollEvent,
		SequenceProjectionNavigateToEntryEvent>;
}
