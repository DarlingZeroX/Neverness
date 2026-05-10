/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Core/SequenceSelectionModel.h"

namespace VisionGal::Editor
{
	void SequenceSelectionModel::SelectSingle(uint32_t index)
	{
		m_selected.clear();
		m_selected.insert(index);
	}

	void SequenceSelectionModel::ToggleSelection(uint32_t index)
	{
		const auto it = m_selected.find(index);
		if (it != m_selected.end())
			m_selected.erase(it);
		else
			m_selected.insert(index);
	}

	void SequenceSelectionModel::Clear()
	{
		m_selected.clear();
	}

	bool SequenceSelectionModel::IsSelected(uint32_t index) const
	{
		return m_selected.find(index) != m_selected.end();
	}

	void SequenceSelectionModel::ClampToSize(size_t entryCount)
	{
		for (auto it = m_selected.begin(); it != m_selected.end();)
		{
			if (*it >= entryCount)
				it = m_selected.erase(it);
			else
				++it;
		}
	}
}
