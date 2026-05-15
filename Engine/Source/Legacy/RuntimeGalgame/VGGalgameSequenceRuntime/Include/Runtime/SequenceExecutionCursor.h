/*
 * SequenceExecutionCursor — 单条「执行帧」在序列剪辑表上的游标
 *
 * Phase 2A 仅使用 SequenceIndex 表示当前剪辑下标；后续可扩展子游标（子轨道、嵌套块内偏移等）
 * 而无需推翻执行器外壳。
 */
#pragma once

#include <cstddef>

#include "../../GSSExport.h"

namespace VisionGal::GalGame
{
	/**
	 * @brief 执行游标：指向 VGSSequenceDataContainer::m_Sequence 中的条目。
	 */
	struct VG_GSS_API SequenceExecutionCursor
	{
		/// 当前剪辑在序列数组中的索引（0 起始）。
		std::size_t SequenceIndex = 0;
	};
}
