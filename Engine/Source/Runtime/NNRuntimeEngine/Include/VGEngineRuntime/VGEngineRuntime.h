#pragma once

#include "NNRuntimeEngine/AsyncSystem.h"
#include "NNRuntimeEngine/AssetRegistrySubsystem.h"
#include "NNRuntimeEngine/AssetSubsystem.h"
#include "NNRuntimeEngine/EntitySubsystem.h"
#include "NNRuntimeEngine/ObjectSubsystem.h"
#include "NNRuntimeEngine/RuntimeScheduler/RuntimeScheduler.h"
#include "NNRuntimeEngine/SceneSubsystem.h"
#include "NNRuntimeEngine/TimingSystem.h"

namespace visiongal::engine
{
/**
 * @brief 行程級引擎 Runtime Facade（Phase 4 + **§2.7.1 Kernel 首包**）：**Initialize → Tick → Shutdown** 由宿主單執行緒驅動。
 *
 * 契約：
 * - `Tick` 僅允許在與 `Initialize` / `Shutdown` 相同之控制執行緒呼叫（測試與未來 game loop）。
 * - `Shutdown` 會阻塞直至 **AsyncSystem** 背景工作 join 完成；請勿於 Async 回呼內呼叫 `Shutdown`。
 *
 * **Entity 子系統（P0 Kernel 首包）**
 * - **`Entity()`** 對應 **`VGEntityAPI`** 之 **`getServiceAbiToken` / `getRuntimeTick`** 轉發載體（見 **VGEngineRuntimeServices**）；**不**承載完整 **VGEntitySystem** ECS，亦**不**與託管 **EntityWorld** 自動同步。
 *
 * **主線（2026，產品文檔）**
 * - **P0-1**：內建 **RuntimeScheduler**（**VGRuntimeScheduler** 首包），在 **TimingSystem::Tick** 之後統一驅動已註冊之 **IRuntimeSubsystem**（當前含 **EntitySubsystem**）。見 **MANAGED_RUNTIME_ARCHITECTURE_AND_PROGRESS.md** **§0.3**。
 */
class VGEngineRuntime final
{
public:
	static VGEngineRuntime& Instance() noexcept;

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

	bool IsInitialized() const noexcept { return initialized_; }

private:
	VGEngineRuntime() noexcept = default;

	bool initialized_{false};
	TimingSystem timing_{};
	AsyncSystem async_{};
	SceneSubsystem scene_{};
	AssetSubsystem asset_{};
	ObjectSubsystem object_{};
	AssetRegistrySubsystem assetRegistry_{};
	EntitySubsystem entity_{};
	RuntimeScheduler scheduler_{};
};
} // namespace visiongal::engine
