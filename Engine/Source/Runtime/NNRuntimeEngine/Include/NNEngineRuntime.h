#pragma once

#include "AsyncSystem.h"
#include "ObjectSubsystem.h"
#include "RuntimeScheduler/RuntimeScheduler.h"
#include "NNRuntimeSceneTickSubsystem.h"
#include "Scene/NNRuntimeScene.h"
#include "Scene/SceneSubsystem.h"
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
	SceneSubsystem& Scene() noexcept { return scene_; }
	ObjectSubsystem& Object() noexcept { return object_; }
	RuntimeScheduler& Scheduler() noexcept { return scheduler_; }

	/** @brief Phase 2+ ECS 场景（NNEntity Handle + entt）；与 SceneSubsystem 并存。 */
	Scene::NNRuntimeScene& EcsScene() noexcept { return ecsScene_; }

	NNRuntimeSceneTickSubsystem& SceneTickSubsystem() noexcept { return sceneTick_; }

	bool IsInitialized() const noexcept { return initialized_; }

private:
	NNEngineRuntime() noexcept = default;

	bool initialized_{false};
	TimingSystem timing_{};
	AsyncSystem async_{};
	SceneSubsystem scene_{};
	ObjectSubsystem object_{};
	RuntimeScheduler scheduler_{};
	Scene::NNRuntimeScene ecsScene_{};
	NNRuntimeSceneTickSubsystem sceneTick_{&ecsScene_};
};
} // namespace NN::Runtime::engine
