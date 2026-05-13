/*
 * GalGameEngineAccess — 当前 Gal 引擎指针（thread_local，仅供 Lua / 编辑器等迁移期入口）
 *
 * 由 GalGameEngine::Initialize / 析构（若将来补充）维护；新逻辑优先显式传递 ISubsystemBus*。
 */

#pragma once
#include "../VGGRCExport.h"

namespace VisionGal::GalGame
{
	class IGalGameEngine;

	struct VG_RUNTIME_GALCORE_API GalGameEngineAccess
	{
		static IGalGameEngine* Current() noexcept;
		static void SetCurrent(IGalGameEngine* engine) noexcept;
	};
}
