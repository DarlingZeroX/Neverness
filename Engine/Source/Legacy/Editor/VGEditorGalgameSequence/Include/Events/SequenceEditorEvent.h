/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "Runtime/SequenceRuntimePropertySnapshot.h"

#include <cstdint>
#include <optional>
#include <string>
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
		RuntimeDebugStream,
		SearchFilterChanged,
	};

	struct SequenceRuntimeStateEventPayload
	{
		bool ControllerOk = false;
		bool ReachedTarget = false;
		unsigned HighlightIndex = 0;
	};

	enum class SequenceRuntimeStreamEventKind : uint8_t
	{
		None = 0,
		RuntimeStarted,
		RuntimePaused,
		RuntimeResumed,
		RuntimeStepped,
		RuntimeNodeEntered,
		RuntimeNodeExited,
		RuntimeFinished,
		RuntimeError,
		RuntimeBreakpointHit,
		/// Phase 10-F：单属性变化观测（载荷见 `PropertyWatch`）。
		RuntimePropertyChanged,
	};

	struct SequenceRuntimeStreamEventPayload
	{
		SequenceRuntimeStreamEventKind Kind = SequenceRuntimeStreamEventKind::None;
		unsigned Index = 0;
		std::string Message;
		bool ControllerOk = false;
		bool ReachedTarget = false;
		std::optional<SequenceRuntimePropertySnapshot> PropertyWatch;
	};

	struct SequenceEditorEvent
	{
		SequenceEditorEventType Type = SequenceEditorEventType::DocumentChanged;
		SequenceDocumentMutationSummary DocumentChanged{};
		/// Phase 6: optional fine-grained transaction (coarse v1 from mutation summary).
		std::optional<SequenceTransaction> CommittedTransaction{};
		SequenceRuntimeStateEventPayload Runtime{};
		SequenceRuntimeStreamEventPayload RuntimeStream{};
	};
}
