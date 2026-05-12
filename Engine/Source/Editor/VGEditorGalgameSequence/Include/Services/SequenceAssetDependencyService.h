/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include <string>

namespace VisionGal::Editor
{
	class SequenceDocument;
	class SequenceDependencyGraph;
	class SequenceValidationCacheService;

	/// Bridges external asset lifecycle to validation / presentation refresh (v1: explicit host calls).
	class SequenceAssetDependencyService
	{
	public:
		void Bind(
			SequenceDocument* document,
			SequenceDependencyGraph* graph,
			SequenceValidationCacheService* validationCache,
			void (*requestPresentationRefresh)(void*),
			void* requestPresentationUserData);

		void OnAssetChanged(const std::string& assetPath);
		void OnAssetRemoved(const std::string& assetPath);
		void OnAssetRenamed(const std::string& oldPath, const std::string& newPath);

	private:
		void InvalidateReferencingEntries(const std::string& assetPath);

		SequenceDocument* m_document = nullptr;
		SequenceDependencyGraph* m_graph = nullptr;
		SequenceValidationCacheService* m_validationCache = nullptr;
		void (*m_requestPresentationRefresh)(void*) = nullptr;
		void* m_requestPresentationUserData = nullptr;
	};
}
