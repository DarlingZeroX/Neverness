/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace VisionGal::Editor
{
	class SequenceDocument;

	/// Asset path → entry indices referencing it (Phase 6).
	class SequenceDependencyGraph
	{
	public:
		void Clear();
		void RebuildFromDocument(const SequenceDocument& document);
		void NoteReference(const std::string& assetPath, unsigned entryIndex);
		[[nodiscard]] std::vector<unsigned> EntriesReferencing(const std::string& assetPath) const;

	private:
		std::unordered_map<std::string, std::vector<unsigned>> m_pathToEntries;
	};
}
