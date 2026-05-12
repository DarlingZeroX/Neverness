/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "Transactions/SequenceTransactionTypes.h"

namespace VisionGal::Editor
{
	/// Payload for document / undo mutations (editor main thread only).
	struct SequenceDocumentMutationSummary
	{
		std::vector<unsigned> TouchedIndices;
		bool StructuralChange = false;
	};

	enum class SequenceEditorEventType : uint8_t
	{
		DocumentChanged,
		/// Legacy name from early editor drafts; same payload as DocumentChanged.
		DocumentMutated = DocumentChanged,
		SelectionChanged,
		ValidationUpdated,
		RuntimeStateChanged,
		SearchFilterChanged,
	};

	struct SequenceRuntimeStateEventPayload
	{
		bool ControllerOk = false;
		bool ReachedTarget = false;
		unsigned HighlightIndex = 0;
	};

	struct SequenceEditorEvent
	{
		SequenceEditorEventType Type = SequenceEditorEventType::DocumentChanged;
		SequenceDocumentMutationSummary DocumentChanged{};
		/// Phase 6: optional fine-grained transaction (coarse v1 from mutation summary).
		std::optional<SequenceTransaction> CommittedTransaction{};
		SequenceRuntimeStateEventPayload Runtime{};
	};
}
