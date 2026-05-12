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

#include <string>
#include <VGCore/Interface/Interface.h>
#include "../GSSExport.h"

namespace VisionGal::GalGame
{
	/**
	* @brief Sequence 播放器生命周期状态。
	*/
	enum class ESSSequenceExecutorState
	{
		Stopped,
		Playing,
		Paused,
		Finished
	};

	/**
	 * @brief 轻量调试快照（Editor / Debugger / Runtime Inspector）。
	 */
	struct VG_GSS_API SSSequenceRuntimeDebugInfo : public IRuntimeInterface
	{
		std::size_t CurrentIndex = 0;
		std::string CurrentComponentType;
		bool Waiting = false;
	};

	/**
	 * @brief Phase 2G：扩展 Inspector 数据（栈、并行、Tick 计数、帧轨迹摘要）。
	 */
	struct VG_GSS_API SSSequenceRuntimeInspectorInfo : public IRuntimeInterface
	{
		std::size_t CurrentIndex = 0;
		std::string CurrentComponentType;
		bool Waiting = false;

		std::uint64_t GlobalTickCounter = 0;
		std::size_t FrameStackDepth = 0;
		std::vector<std::size_t> FrameCursors;
		std::vector<bool> FrameWaiting;

		bool InParallelGroup = false;
		std::string ParallelPolicy;
		std::vector<std::size_t> ParallelActiveIndices;

		/// 最近若干 Tick 的环形摘要（旧 → 新）。
		std::vector<std::string> FrameTraceLines;
	};
}
