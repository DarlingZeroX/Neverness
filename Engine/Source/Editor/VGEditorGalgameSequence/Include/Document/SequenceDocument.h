/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include <HCore/Interface/HConfig.h>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "VGGalgameScriptSequence/Include/Sequence/DataContainer.h"

namespace VisionGal::Editor
{
	/// Owns asset path, dirty flag, and the edited sequence (thin wrapper over `VGSSequenceDataContainer`).
	class SequenceDocument
	{
	public:
		SequenceDocument();

		/// LEGACY: direct access for UI not yet migrated behind commands.
		Ref<VisionGal::VGSSequenceDataContainer> GetSequence() const { return m_sequence; }

		const std::string& GetAssetPath() const { return m_assetPath; }

		bool IsDirty() const { return m_dirty; }
		void MarkDirty();
		void ClearDirty();

		/// Creates a new empty sequence container.
		void ResetSequenceData();

		/// Fills default demo entries (matches previous editor ctor behaviour).
		void FillDefaultDemoEntries();

		bool LoadFromAssetPath(const std::string& path);
		bool SaveToAssetPath();

		/// Sets `m_assetPath` then saves. Returns false if `path` is empty or write fails.
		bool SaveAsToAssetPath(const std::string& path);

		/// Clears asset path and replaces sequence with an empty container (untitled new document).
		void ResetToUntitledEmpty();

		void AddEntryByTypeNameID(const std::string& typeNameID);
		void RemoveEntries(const std::vector<unsigned>& indices);
		bool ReorderInsertBefore(unsigned sourceIndex, unsigned targetIndex);
		bool ReorderInsertBehind(unsigned sourceIndex, unsigned targetIndex);

		/// Restores full ordered entry list (undo helpers / move snapshot).
		void SetSequenceEntries(const std::vector<Ref<VisionGal::IVGSSequenceComponent>>& entries)
		{
			m_sequence->m_Sequence = entries;
			for (unsigned i = 0; i < m_sequence->m_Sequence.size(); ++i)
				m_sequence->m_Sequence[i]->SequenceIndex = i;
			MarkDirty();
		}

		/// Inserts a single entry at `index` (clamped to end). Reindexes `SequenceIndex`.
		void InsertEntryAt(unsigned index, Ref<VisionGal::IVGSSequenceComponent> entry)
		{
			auto& vec = m_sequence->m_Sequence;
			if (index > vec.size())
				index = static_cast<unsigned>(vec.size());
			vec.insert(vec.begin() + static_cast<std::ptrdiff_t>(index), std::move(entry));
			for (unsigned i = 0; i < vec.size(); ++i)
				vec[i]->SequenceIndex = i;
			MarkDirty();
		}

	private:
		std::string m_assetPath;
		Ref<VisionGal::VGSSequenceDataContainer> m_sequence;
		bool m_dirty = false;
	};
}
