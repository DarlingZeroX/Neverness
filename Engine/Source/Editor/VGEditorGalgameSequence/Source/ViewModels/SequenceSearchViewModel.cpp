/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "ViewModels/SequenceSearchViewModel.h"

#include "ViewModels/SequenceEntryViewModel.h"

namespace VisionGal::Editor
{
	bool SequenceSearchViewModel::RowPassesFilters(const SequenceEntryViewModel& row) const
	{
		if ((m_dimensions & static_cast<uint32_t>(Dimension::ValidationErrorsOnly)) != 0 && !row.HasValidationError)
			return false;
		if ((m_dimensions & static_cast<uint32_t>(Dimension::RuntimeLineOnly)) != 0 && !row.RuntimeHighlight)
			return false;

		if (m_textFilter.empty())
			return true;

		if ((m_dimensions & static_cast<uint32_t>(Dimension::TextMatch)) != 0)
		{
			const std::string& f = m_textFilter;
			const bool hit = row.DisplayName.find(f) != std::string::npos || row.TypeNameID.find(f) != std::string::npos
				|| row.Subtitle.find(f) != std::string::npos || row.Category.find(f) != std::string::npos;
			if (!hit)
				return false;
		}

		if ((m_dimensions & static_cast<uint32_t>(Dimension::TypeName)) != 0)
		{
			if (row.TypeNameID.find(m_textFilter) == std::string::npos)
				return false;
		}

		if ((m_dimensions & static_cast<uint32_t>(Dimension::Category)) != 0)
		{
			if (row.Category.find(m_textFilter) == std::string::npos)
				return false;
		}

		return true;
	}
}
