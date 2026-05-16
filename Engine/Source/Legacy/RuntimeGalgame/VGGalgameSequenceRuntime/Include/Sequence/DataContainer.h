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
#pragma once
#include <vector>
#include <NNKernel/Interface/HConfig.h>
#include "../../GSSExport.h"
#include "../../Interface/IVGSSequenceComponent.h"

namespace VisionGal
{
	struct VG_GSS_API VGSSequenceDataContainer
	{
		void AppendEntry(const Ref<IVGSSequenceComponent>& item);
		bool RemoveEntry(unsigned index);
		bool RemoveEntries(const std::vector<unsigned>& indices);
		void ClearEntries() { m_Sequence.clear(); }
		bool IsEmpty() const { return m_Sequence.empty(); }
		bool IsValidIndex(unsigned index) const { return index < m_Sequence.size(); }
		bool SwapEntries(unsigned index1, unsigned index2);
		bool InsertBefore(unsigned source, unsigned target);
		bool InsertBehind(unsigned source, unsigned target);
		bool InsertBefore(unsigned sourceBegin, unsigned sourceEnd, unsigned target);
		bool InsertBehind(unsigned sourceBegin, unsigned sourceEnd, unsigned target);

		std::vector<Ref<IVGSSequenceComponent>> m_Sequence;
	};

	void AddVGSSequenceDataContainerDefaultEntries(VGSSequenceDataContainer& container);
}
