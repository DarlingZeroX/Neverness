/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Services/SequenceAssetDependencyService.h"

#include "AssetMonitoring/SequenceDependencyGraph.h"
#include "Document/SequenceDocument.h"
#include "Services/SequenceValidationCacheService.h"

namespace VisionGal::Editor
{
	void SequenceAssetDependencyService::Bind(
		SequenceDocument* document,
		SequenceDependencyGraph* graph,
		SequenceValidationCacheService* validationCache,
		void (*requestPresentationRefresh)(void*),
		void* requestPresentationUserData)
	{
		m_document = document;
		m_graph = graph;
		m_validationCache = validationCache;
		m_requestPresentationRefresh = requestPresentationRefresh;
		m_requestPresentationUserData = requestPresentationUserData;
	}

	void SequenceAssetDependencyService::InvalidateReferencingEntries(const std::string& assetPath)
	{
		if (m_document == nullptr || m_graph == nullptr || m_validationCache == nullptr)
			return;
		m_graph->RebuildFromDocument(*m_document);
		const std::vector<unsigned> touched = m_graph->EntriesReferencing(assetPath);
		if (touched.empty())
			return;
		m_validationCache->NotifyEntriesPropertyTouch(touched);
		if (m_requestPresentationRefresh != nullptr)
			m_requestPresentationRefresh(m_requestPresentationUserData);
	}

	void SequenceAssetDependencyService::OnAssetChanged(const std::string& assetPath)
	{
		InvalidateReferencingEntries(assetPath);
	}

	void SequenceAssetDependencyService::OnAssetRemoved(const std::string& assetPath)
	{
		InvalidateReferencingEntries(assetPath);
	}

	void SequenceAssetDependencyService::OnAssetRenamed(const std::string& oldPath, const std::string& newPath)
	{
		(void)newPath;
		InvalidateReferencingEntries(oldPath);
	}
}
