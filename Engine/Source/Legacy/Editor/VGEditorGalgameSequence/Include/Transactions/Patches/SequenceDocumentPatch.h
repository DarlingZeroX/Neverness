/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "Schema/SequencePropertyValue.h"

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

namespace VisionGal::Editor
{
	enum class SequenceDocumentPatchKind : uint8_t
	{
		InsertEntry = 0,
		RemoveEntry,
		MoveEntry,
		SetProperty,
	};

	struct SequenceInsertEntryPatch
	{
		unsigned AtIndex = 0;
		std::string TypeNameID;
	};

	struct SequenceRemoveEntryPatch
	{
		unsigned Index = 0;
	};

	struct SequenceMoveEntryPatch
	{
		unsigned FromIndex = 0;
		unsigned ToIndex = 0;
	};

	struct SequenceSetPropertyPatch
	{
		unsigned EntryIndex = 0;
		std::string PropertyPath;
		/// Phase 10-E：类型化新值；字符串属性仍使用 `std::string` 分支。
		SequencePropertyValue Value;
	};

	using SequenceDocumentPatch = std::variant<
		SequenceInsertEntryPatch,
		SequenceRemoveEntryPatch,
		SequenceMoveEntryPatch,
		SequenceSetPropertyPatch>;

	struct SequencePatchTransactionV2
	{
		SequenceDocumentPatchKind SummaryKind = SequenceDocumentPatchKind::SetProperty;
		std::vector<SequenceDocumentPatch> Patches;
	};
}
