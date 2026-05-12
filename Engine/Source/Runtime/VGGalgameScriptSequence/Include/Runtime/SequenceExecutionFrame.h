/*
 * SequenceExecutionFrame — 单层序列执行状态（栈中的一帧）
 *
 * 未来将用于子序列调用、分支并行等：ExecutionInstance 维护 vector 栈，活动帧为栈顶。
 * Phase 2A 线性播放仅维持单帧，但字段布局与后续 Push/Pop 扩展一致。
 */
#pragma once

#include "SequenceExecutionCursor.h"

#include "../../GSSExport.h"

namespace VisionGal::GalGame
{
	/**
	 * @brief 单帧运行时状态：游标 + 是否已对本条执行过 Execute + Wait 栅栏。
	 */
	struct VG_GSS_API SequenceExecutionFrame
	{
		/// 本帧在序列上的读位置。
		SequenceExecutionCursor Cursor;

		/// 当前游标处是否已执行过 Execute（Wait 期间保持 true，与旧 m_HasDispatchedCurrentClip 一致）。
		bool HasDispatchedCurrentClip = false;

		/// 是否处于 Wait（对话等待、动画门闩等），与旧 m_PlaybackWaiting 一致。
		bool Waiting = false;
	};
}
