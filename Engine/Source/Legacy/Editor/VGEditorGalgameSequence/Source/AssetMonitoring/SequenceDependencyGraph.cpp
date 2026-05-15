/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "AssetMonitoring/SequenceDependencyGraph.h"

#include "Document/SequenceDocument.h"

#include "VGGalgameSequenceRuntime/Include/Sequence/Components.h"
#include "VGGalgameSequenceRuntime/Interface/IVGSSequenceComponent.h"

#include <algorithm>

namespace VisionGal::Editor
{
	void SequenceDependencyGraph::Clear()
	{
		m_pathToEntries.clear();
	}

	void SequenceDependencyGraph::NoteReference(const std::string& assetPath, const unsigned entryIndex)
	{
		if (assetPath.empty())
			return;
		auto& vec = m_pathToEntries[assetPath];
		vec.push_back(entryIndex);
		std::sort(vec.begin(), vec.end());
		vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
	}

	std::vector<unsigned> SequenceDependencyGraph::EntriesReferencing(const std::string& assetPath) const
	{
		const auto it = m_pathToEntries.find(assetPath);
		if (it == m_pathToEntries.end())
			return {};
		return it->second;
	}

	void SequenceDependencyGraph::RebuildFromDocument(const SequenceDocument& document)
	{
		Clear();
		const unsigned n = document.GetEntryCount();
		for (unsigned i = 0; i < n; ++i)
		{
			const auto* c = document.GetEntryAt(i);
			if (c == nullptr)
				continue;
			if (const auto* fig = dynamic_cast<const VisionGal::VGSSC_ChangeFigure*>(c))
			{
				if (!fig->TextureResourcePath.empty())
					NoteReference(fig->TextureResourcePath, i);
			}
			else if (const auto* bg = dynamic_cast<const VisionGal::VGSSC_ChangeBackground*>(c))
			{
				if (!bg->TextureResourcePath.empty())
					NoteReference(bg->TextureResourcePath, i);
			}
		}
	}
}
