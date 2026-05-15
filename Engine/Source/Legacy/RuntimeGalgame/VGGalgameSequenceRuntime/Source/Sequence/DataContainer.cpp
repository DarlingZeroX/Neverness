/*
* This source file is part of VisionGal, the Visual Novel Engine
*
* For the latest information, see https://darlingzerox.github.io/VisionGalDoc/
* GitHub page: https://github.com/DarlingZeroX/VisionGal
*
* Copyright (c) 2025-present 梦旅缘心
*
* See the LICENSE file in the project root for details.
*/

#include "Sequence/DataContainer.h"
#include "Sequence/Components.h"
#include <algorithm>

namespace VisionGal
{
	void VGSSequenceDataContainer::AppendEntry(const Ref<IVGSSequenceComponent>& item)
	{
		item->SequenceIndex = static_cast<unsigned>(m_Sequence.size());
		m_Sequence.push_back(item);
	}

	bool VGSSequenceDataContainer::RemoveEntry(unsigned index)
	{
		if (index >= m_Sequence.size())
			return false;

		m_Sequence.erase(m_Sequence.begin() + index);
		return true;
	}

	bool VGSSequenceDataContainer::RemoveEntries(const std::vector<unsigned>& indices)
	{
		// 先对 indices 进行排序和去重
		std::vector<unsigned> sortedIndices = indices;
		std::sort(sortedIndices.begin(), sortedIndices.end());
		sortedIndices.erase(std::unique(sortedIndices.begin(), sortedIndices.end()), sortedIndices.end());
		// 从后往前删除，避免删除时索引变化导致的问题
		for (auto it = sortedIndices.rbegin(); it != sortedIndices.rend(); ++it)
		{
			if (*it < m_Sequence.size())
			{
				m_Sequence.erase(m_Sequence.begin() + *it);
			}
		}
		return true;
	}

	bool VGSSequenceDataContainer::SwapEntries(unsigned source, unsigned target)
	{
		if (!IsValidIndex(source) || !IsValidIndex(target) || source == target)
			return false;
		std::swap(m_Sequence[source], m_Sequence[target]);
		return true;
	}

	bool VGSSequenceDataContainer::InsertBefore(unsigned source, unsigned target)
	{
		if (!IsValidIndex(source) || !IsValidIndex(target) || source == target)
			return false;
		auto entry = m_Sequence[source];
		m_Sequence.erase(m_Sequence.begin() + source);
		if (source < target) --target;
		m_Sequence.insert(m_Sequence.begin() + target, entry);
		return true;
	}

	bool VGSSequenceDataContainer::InsertBehind(unsigned source, unsigned target)
	{
		if (!IsValidIndex(source) || !IsValidIndex(target) || source == target)
			return false;
		auto entry = m_Sequence[source];
		//if (source < target) ++target;
		m_Sequence.insert(m_Sequence.begin() + target + 1, entry);
		m_Sequence.erase(m_Sequence.begin() + source);
		return true;
	}

	bool VGSSequenceDataContainer::InsertBefore(unsigned sourceBegin, unsigned sourceEnd, unsigned target)
	{
		return true;
	}

	bool VGSSequenceDataContainer::InsertBehind(unsigned sourceBegin, unsigned sourceEnd, unsigned target)
	{
		return true;
	}

	void AddVGSSequenceDataContainerDefaultEntries(VGSSequenceDataContainer& container)
	{
		auto dialogue1 = MakeRef<VGSSC_CommonDialogue>();
		container.AppendEntry(dialogue1);

		auto dialogue2 = MakeRef<VGSSC_CommonDialogue>();
		container.AppendEntry(dialogue2);
	}
}
