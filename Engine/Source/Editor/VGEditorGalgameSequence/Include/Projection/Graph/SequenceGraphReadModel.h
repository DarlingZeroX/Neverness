/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace VisionGal::Editor
{
	/// Authoring-only graph node (linear sequence index); not an execution VM node.
	struct SequenceGraphNodeVM
	{
		uint32_t EntryIndex = 0;
		std::string Title;
		std::string Subtitle;
		std::string TypeNameID;
		float LayoutX = 0.f;
		float LayoutY = 0.f;
	};

	enum class SequenceGraphEdgeKind : uint8_t
	{
		LinearNext = 0,
	};

	/// Visual edge between authoring nodes (default: linear flow i → i+1).
	struct SequenceGraphEdgeVM
	{
		uint32_t FromEntryIndex = 0;
		uint32_t ToEntryIndex = 0;
		SequenceGraphEdgeKind Kind = SequenceGraphEdgeKind::LinearNext;
	};
}
