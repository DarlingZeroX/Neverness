/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include <HCore/Interface/HConfig.h>
#include <cstdint>
#include <vector>

namespace VisionGal
{
	class IVGSSequenceComponent;
}

namespace VisionGal::Editor
{
	struct SequenceEditorContext;

	/// Editor clipboard: deep copies of selected entries for cut/copy/paste with undo.
	class SequenceClipboard
	{
	public:
		void Clear();

		/// Replaces clipboard with clones of the given sorted unique indices (ascending).
		void SetFromSelection(const SequenceEditorContext& context, const std::vector<uint32_t>& sortedUniqueIndices);

		/// Copy selection to clipboard (no document change).
		void CopySelection(SequenceEditorContext& context);

		/// Copy then remove selected entries (single undo via remove command — use Compound with paste for full cut semantics).
		void CutSelection(SequenceEditorContext& context);

		/// Inserts cloned clipboard entries at the computed insert index (undoable).
		void TryPaste(SequenceEditorContext& context);

		bool HasContent() const { return !m_entries.empty(); }

	private:
		static unsigned ComputePasteInsertIndex(const SequenceEditorContext& context);

		std::vector<Ref<VisionGal::IVGSSequenceComponent>> m_entries;
	};
}
