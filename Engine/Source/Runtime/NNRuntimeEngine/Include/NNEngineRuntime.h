#pragma once

#include "AsyncSystem.h"
#include "AssetRegistrySubsystem.h"
#include "AssetSubsystem.h"
#include "EntitySubsystem.h"
#include "ObjectSubsystem.h"
#include "RuntimeScheduler/RuntimeScheduler.h"
#include "NNRuntimeSceneTickSubsystem.h"
#include "Scene/NNRuntimeScene.h"
#include "SceneSubsystem.h"
#include "TimingSystem.h"

namespace NN::Runtime::engine
{
/**
 * @brief 行程級引擎 Runtime Facade（Phase 4 + **§2.7.1 Kernel 首包**）：**Initialize → Tick → Shutdown** 由宿主單執行緒驅動。
 *
 * 契約：
 * - `Tick` 僅允許在與 `Initialize` / `Shutdown` 相同之控制執行緒呼叫（測試與未來 game loop）。
 * - `Shutdown` 會阻塞直至 **AsyncSystem** 背景工作 join 完成；請勿於 Async 回呼內呼叫 `Shutdown`。
 *
 * **Entity 子系統（P0 Kernel 首包）**
 * - **`Entity()`** 對應 **`NNEntityAPI`** 之 **`getServiceAbiToken` / `getRuntimeTick`** 轉發載體（見 **VGEngineRuntimeServices**）；**不**承載完整 **VGEntitySystem** ECS，亦**不**與託管 **EntityWorld** 自動同步。
 *
 * **主線（2026，產品文檔）**
 * - **P0-1**：內建 **RuntimeScheduler**（**VGRuntimeScheduler** 首包），在 **TimingSystem::Tick** 之後統一驅動已註冊之 **IRuntimeSubsystem**（當前含 **EntitySubsystem**）。見 **MANAGED_RUNTIME_ARCHITECTURE_AND_PROGRESS.md** **§0.3**。
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
	AssetSubsystem& Asset() noexcept { return asset_; }
	ObjectSubsystem& Object() noexcept { return object_; }
	AssetRegistrySubsystem& AssetRegistry() noexcept { return assetRegistry_; }
	EntitySubsystem& Entity() noexcept { return entity_; }
	RuntimeScheduler& Scheduler() noexcept { return scheduler_; }

	/** @brief Phase 2+ ECS 场景（**NNEntity** Handle + entt）；与 **SceneSubsystem** 并存。 */
	Scene::NNRuntimeScene& EcsScene() noexcept { return ecsScene_; }

	NNRuntimeSceneTickSubsystem& SceneTickSubsystem() noexcept { return sceneTick_; }

	bool IsInitialized() const noexcept { return initialized_; }

private:
	NNEngineRuntime() noexcept = default;

	bool initialized_{false};
	TimingSystem timing_{};
	AsyncSystem async_{};
	SceneSubsystem scene_{};
	AssetSubsystem asset_{};
	ObjectSubsystem object_{};
	AssetRegistrySubsystem assetRegistry_{};
	EntitySubsystem entity_{};
	RuntimeScheduler scheduler_{};
	Scene::NNRuntimeScene ecsScene_{};
	NNRuntimeSceneTickSubsystem sceneTick_{&ecsScene_};
};
} // namespace NN::Runtime::engine
