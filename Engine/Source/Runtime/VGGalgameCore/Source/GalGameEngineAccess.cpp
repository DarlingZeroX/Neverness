/*
 * GalGameEngineAccess 实现
 */

#include "GalGameEngineAccess.h"

#include "IGameEngine.h"
#include "GalExecutionLifecycle.h"

static_assert(static_cast<int>(VisionGal::GalGame::ExecutionLifecycle::Finished) <= 5, "GalExecutionLifecycle enum range");

namespace VisionGal::GalGame
{
	namespace
	{
		thread_local IGalGameEngine* gCurrentGalGameEngine = nullptr;
	}

	IGalGameEngine* GalGameEngineAccess::Current() noexcept
	{
		return gCurrentGalGameEngine;
	}

	void GalGameEngineAccess::SetCurrent(IGalGameEngine* engine) noexcept
	{
		gCurrentGalGameEngine = engine;
	}
}
