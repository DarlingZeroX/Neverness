/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Core/SequenceClipboard.h"

#include "Commands/PasteSequenceEntriesCommand.h"
#include "Commands/RemoveSequenceEntryCommand.h"
#include "Core/SequenceEditorContext.h"
#include "Core/SequenceSelectionModel.h"
#include "Document/SequenceDocument.h"

#include "VGGalgameSequenceRuntime/Interface/IVGSSequenceComponent.h"

#include <algorithm>

namespace VisionGal::Editor
{
	void SequenceClipboard::Clear()
	{
		m_entries.clear();
	}

	void SequenceClipboard::SetFromSelection(const SequenceEditorContext& context, const std::vector<uint32_t>& sortedUniqueIndices)
	{
		m_entries.clear();
		if (context.document == nullptr)
			return;
		for (uint32_t idx : sortedUniqueIndices)
		{
			auto* comp = context.document->GetEntryAt(static_cast<unsigned>(idx));
			if (comp == nullptr)
				continue;
			m_entries.push_back(comp->Clone());
		}
	}

	static std::vector<uint32_t> SelectionToSortedVector(const std::unordered_set<uint32_t>& sel)
	{
		std::vector<uint32_t> v(sel.begin(), sel.end());
		std::sort(v.begin(), v.end());
		v.erase(std::unique(v.begin(), v.end()), v.end());
		return v;
	}

	void SequenceClipboard::CopySelection(SequenceEditorContext& context)
	{
		if (context.document == nullptr || context.selection == nullptr)
			return;
		SetFromSelection(context, SelectionToSortedVector(context.selection->GetSelection()));
	}

	void SequenceClipboard::CutSelection(SequenceEditorContext& context)
	{
		if (context.document == nullptr || context.selection == nullptr || context.undo == nullptr)
			return;
		const std::vector<uint32_t> sorted = SelectionToSortedVector(context.selection->GetSelection());
		if (sorted.empty())
			return;
		SetFromSelection(context, sorted);
		std::vector<unsigned> remove;
		remove.reserve(sorted.size());
		for (uint32_t i : sorted)
			remove.push_back(i);
		context.ExecuteCommand(std::make_unique<RemoveSequenceEntryCommand>(std::move(remove)));
	}

	unsigned SequenceClipboard::ComputePasteInsertIndex(const SequenceEditorContext& context)
	{
		if (context.document == nullptr)
			return 0;
		const size_t n = context.document->GetEntryCount();
		if (context.selection == nullptr || context.selection->GetSelection().empty())
			return static_cast<unsigned>(n);
		unsigned maxSel = 0;
		for (uint32_t i : context.selection->GetSelection())
			maxSel = std::max(maxSel, i);
		if (maxSel + 1u > n)
			return static_cast<unsigned>(n);
		return maxSel + 1u;
	}

	void SequenceClipboard::TryPaste(SequenceEditorContext& context)
	{
		if (m_entries.empty() || context.document == nullptr)
			return;
		std::vector<Ref<VisionGal::IVGSSequenceComponent>> protos;
		protos.reserve(m_entries.size());
		for (const auto& e : m_entries)
		{
			if (e != nullptr)
				protos.push_back(e->Clone());
		}
		if (protos.empty())
			return;
		const unsigned at = ComputePasteInsertIndex(context);
		context.ExecuteCommand(std::make_unique<PasteSequenceEntriesCommand>(at, std::move(protos)));
	}
}
