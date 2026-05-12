/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "Transactions/SequenceTransactionTypes.h"

namespace VisionGal::Editor
{
	struct SequenceDocumentMutationSummary;

	[[nodiscard]] SequenceTransaction BuildTransactionFromMutationSummary(
		const SequenceDocumentMutationSummary& summary,
		std::uint64_t generation,
		SequenceTransactionSource source);
}
