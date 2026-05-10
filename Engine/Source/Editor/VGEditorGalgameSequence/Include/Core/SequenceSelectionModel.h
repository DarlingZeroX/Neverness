/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include <cstdint>
#include <unordered_set>
#include <vector>

namespace VisionGal::Editor
{
	class SequenceSelectionModel
	{
	public:
		void SelectSingle(uint32_t index);
		void ToggleSelection(uint32_t index);
		void Clear();
		bool IsSelected(uint32_t index) const;
		const std::unordered_set<uint32_t>& GetSelection() const { return m_selected; }

		/// Drops indices >= entryCount; if empty after clamp, clears selection.
		void ClampToSize(size_t entryCount);

	private:
		std::unordered_set<uint32_t> m_selected;
	};
}
