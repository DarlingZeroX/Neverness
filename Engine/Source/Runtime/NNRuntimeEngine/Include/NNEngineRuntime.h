#pragma once

#include "AsyncSystem.h"
#include "ObjectSubsystem.h"
#include "RuntimeScheduler/RuntimeScheduler.h"
#include "TimingSystem.h"

namespace NN::Runtime::engine
{
/**
 * @brief 行程级引擎 Runtime Facade：Initialize → Tick → Shutdown 由宿主单线程驱动。
 *
 * 已移除：
 * - AssetRegistrySubsystem — 合并到 NNRuntimeAsset 模块（NNAssetRegistry）
 * - AssetSubsystem — 空壳 stub，已由 NNAssetManager 替代
 * - EntitySubsystem — ABI 骨架（getServiceAbiToken + getRuntimeTick），已删除
 * - NNRuntimeScene / SceneSubsystem — 移至 Legacy（C# Friflo ECS 替代）
 * - NNRuntimeSceneTickSubsystem — 随 NNRuntimeScene 一起移除
 */
class NNEngineRuntime final
{
public:
	static NNEngineRuntime& Instance() noexcept;

	bool Initialize() noexcept;
	void Tick(float deltaTimeSeconds) noexcept;
	void Shutdown() noexcept;

	TimingSystem& Timing() noexcept { return timing_; }
	AsyncSystem& Async() noexcept { return async_; }
	ObjectSubsystem& Object() noexcept { return object_; }
	RuntimeScheduler& Scheduler() noexcept { return scheduler_; }

	bool IsInitialized() const noexcept { return initialized_; }

private:
	NNEngineRuntime() noexcept = default;

	bool initialized_{false};
	TimingSystem timing_{};
	AsyncSystem async_{};
	ObjectSubsystem object_{};
	RuntimeScheduler scheduler_{};
};
} // namespace NN::Runtime::engine
