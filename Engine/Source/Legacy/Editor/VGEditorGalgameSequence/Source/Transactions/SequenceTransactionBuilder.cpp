/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Transactions/SequenceTransactionBuilder.h"

#include "Events/SequenceEditorEvent.h"

#include <atomic>
#include <unordered_set>

namespace VisionGal::Editor
{
	namespace
	{
		std::atomic<std::uint64_t> g_nextTransactionId{1};
	}

	SequenceTransaction BuildTransactionFromMutationSummary(
		const SequenceDocumentMutationSummary& summary,
		const std::uint64_t generation,
		const SequenceTransactionSource source)
	{
		SequenceTransaction tx;
		tx.TransactionId = g_nextTransactionId.fetch_add(1, std::memory_order_relaxed);
		tx.Generation = generation;
		tx.Source = source;

		if (summary.StructuralChange)
		{
			tx.Flags |= SequenceTransactionFlags::StructuralChange;
			SequenceMutationRecord r;
			r.Type = SequenceMutationType::Structure;
			r.EntryIndex = -1;
			r.PropertyPath = "sequence.structure";
			tx.Mutations.push_back(std::move(r));
		}

		std::unordered_set<unsigned> uniq(summary.TouchedIndices.begin(), summary.TouchedIndices.end());
		for (unsigned idx : uniq)
		{
			tx.Flags |= SequenceTransactionFlags::PropertyEdit;
			SequenceMutationRecord r;
			r.Type = SequenceMutationType::EntryProperty;
			r.EntryIndex = static_cast<std::int32_t>(idx);
			r.PropertyPath = "entry.property";
			tx.Mutations.push_back(std::move(r));
		}

		tx.Source = source;
		return tx;
	}
}
