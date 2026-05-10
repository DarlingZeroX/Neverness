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
}
