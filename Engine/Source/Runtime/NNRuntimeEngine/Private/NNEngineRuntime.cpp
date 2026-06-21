/**
 * @file NNEngineRuntime.cpp
 * @brief NNEngineRuntime 单例：Initialize / Tick / Shutdown。
 *
 * Tick 顺序：
 * - 先 timing_.Tick，再构造 RuntimeFrameContext，最后 scheduler_.Tick
 *
 * Shutdown：
 * - 先 async_.Shutdown（join 后台等待），再 scheduler_.ShutdownRegistered()。
 *
 * 已移除：
 * - EntitySubsystem（ABI 骨架，已删除）
 * - NNRuntimeScene / SceneSubsystem / NNRuntimeSceneTickSubsystem（移至 Legacy）
 */

#include "NNEngineRuntime.h"

#include "RuntimeScheduler/RuntimeFrameContext.h"

namespace NN::Runtime::engine
{
NNEngineRuntime& NNEngineRuntime::Instance() noexcept
{
	static NNEngineRuntime s_instance;
	return s_instance;
}

bool NNEngineRuntime::Initialize() noexcept
{
	if (initialized_)
	{
		return true;
	}

	timing_.Reset();
	scheduler_.InitializeRegistered();
	initialized_ = true;
	return true;
}

void NNEngineRuntime::Tick(float deltaTimeSeconds) noexcept
{
	if (!initialized_)
	{
		return;
	}

	timing_.Tick(deltaTimeSeconds);
	RuntimeFrameContext frame{};
	frame.deltaTimeSeconds = timing_.GetDeltaTime();
	frame.totalTimeSeconds = timing_.GetTotalTime();
	frame.frameIndex = timing_.GetFrameIndex();
	frame.fixedDeltaTimeSeconds = scheduler_.GetFixedDeltaTimeSeconds();
	scheduler_.Tick(frame);
}

void NNEngineRuntime::Shutdown() noexcept
{
	if (!initialized_)
	{
		return;
	}

	async_.Shutdown();
	scheduler_.ShutdownRegistered();
	initialized_ = false;
}
} // namespace NN::Runtime::engine
