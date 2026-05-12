/*
 * SequenceExecutionFrame — 单层序列执行状态（栈中的一帧）
 *
 * 未来将用于子序列调用、分支并行等：ExecutionInstance 维护 vector 栈，活动帧为栈顶。
 *
 * Phase 2C/2F：
 * - ActiveWaitTokenIds：线性播放下当前剪辑挂起的异步闸门（空表示不等待）；
 * - ParallelGroup：若 engaged，则本帧走并行 Tick 路径，线性游标字段在组运行期间作占位。
 */
#pragma once

#include "SequenceExecutionCursor.h"
#include "SequenceParallelGroup.h"

#include <cstdint>
#include <optional>
#include <vector>

#include "../../GSSExport.h"

namespace VisionGal::GalGame
{
	/**
	 * @brief 单帧运行时状态：游标 + 派发标记 + 线性等待 token + 可选并行组。
	 */
	struct VG_GSS_API SequenceExecutionFrame
	{
		/// 本帧在序列上的读位置（线性模式；并行模式下可表示组外游标或占位）。
		SequenceExecutionCursor Cursor;

		/// 当前游标处是否已执行过 Execute（线性模式；并行槽位使用 ParallelGroup.SlotHasDispatched）。
		bool HasDispatchedCurrentClip = false;

		/// 线性模式：挂起的 WaitToken id；全部 IsResolved 后本帧可继续前进。
		std::vector<std::uint64_t> ActiveWaitTokenIds;

		/// 若 set，本 Tick 走并行槽调度；与 ActiveWaitTokenIds 互斥使用（并行等待走 SlotWaitTokens）。
		std::optional<SequenceParallelGroup> ParallelGroup;
	};
}
