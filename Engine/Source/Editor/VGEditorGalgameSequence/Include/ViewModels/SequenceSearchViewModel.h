/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include <cstdint>
#include <string>

namespace VisionGal::Editor
{
	struct SequenceEntryViewModel;

	/// Search / filter dimensions for `SequenceDocumentViewModel` visibility.
	/// 用于 `SequenceDocumentViewModel` 可见性筛选的搜索维度。
	class SequenceSearchViewModel
	{
	public:
		enum class Dimension : uint32_t
		{
			None = 0,
			TextMatch = 1u << 0,
			TypeName = 1u << 1,
			Category = 1u << 2,
			ValidationErrorsOnly = 1u << 3,
			RuntimeLineOnly = 1u << 4,
		};

		std::string& TextFilter() { return m_textFilter; }
		const std::string& TextFilter() const { return m_textFilter; }

		uint32_t ActiveDimensions() const { return m_dimensions; }
		void SetActiveDimensions(uint32_t mask) { m_dimensions = mask; }
		void ToggleDimension(Dimension d)
		{
			const uint32_t bit = static_cast<uint32_t>(d);
			m_dimensions ^= bit;
		}
		bool HasDimension(Dimension d) const
		{
			return (m_dimensions & static_cast<uint32_t>(d)) != 0;
		}

		/// Row visible when all enabled dimension predicates pass (text may be empty = match all).
		/// 当所有已启用维度谓词通过时行可见（文本为空则视为全部匹配）。
		bool RowPassesFilters(const SequenceEntryViewModel& row) const;

	private:
		std::string m_textFilter;
		uint32_t m_dimensions = static_cast<uint32_t>(Dimension::TextMatch);
	};
}

