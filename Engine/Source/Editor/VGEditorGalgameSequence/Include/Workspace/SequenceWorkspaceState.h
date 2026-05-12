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

namespace VisionGal::Editor
{
	/// Persists simple per-window visibility (Phase 7 workspace v1).
	class SequenceWorkspaceState
	{
	public:
		SequenceWorkspaceState();
		~SequenceWorkspaceState();

		[[nodiscard]] bool IsWindowVisible(const char* key) const;
		void SetWindowVisible(const char* key, bool visible);

	private:
		void Load();
		void Save() const;

		std::unordered_map<std::string, bool> m_visible;
		std::string m_storagePath;
	};
}
