/*
 * SequenceParallelGroup — 同一逻辑帧内并行推进的多条序列下标
 *
 * ActiveIndices：指向 VGSSequenceDataContainer::m_Sequence 的下标列表（去重由调用方保证）。
 * Policy：WaitAll / WaitAny 汇合语义（见 SequenceBlockingPolicy）。
 * SlotHasDispatched / SlotWaitTokens：与 ActiveIndices 等长，描述每条并行槽的派发与等待。
 * ResumeSequenceIndex：组完成后主游标写入的位置（通常为 max(ActiveIndices)+1）。
 */
#pragma once

#include "SequenceBlockingPolicy.h"

#include <cstddef>
#include <cstdint>
#include <vector>

#include "../../GSSExport.h"

namespace VisionGal::GalGame
{
	struct VG_GSS_API SequenceParallelGroup
	{
		std::vector<std::size_t> ActiveIndices;
		SequenceBlockingPolicy Policy = SequenceBlockingPolicy::WaitAll;

		/// 与 ActiveIndices 一一对应：该槽当前剪辑是否已 Execute。
		std::vector<bool> SlotHasDispatched;

		/// 与 ActiveIndices 一一对应：各槽挂起的 WaitToken id（空表示该槽无等待）。
		std::vector<std::vector<std::uint64_t>> SlotWaitTokens;

		/// 并行组结束后主线程游标跳转目标。
		std::size_t ResumeSequenceIndex = 0;
	};
}
