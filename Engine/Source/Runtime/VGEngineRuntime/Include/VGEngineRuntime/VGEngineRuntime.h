#pragma once

#include "VGEngineRuntime/AsyncSystem.h"
#include "VGEngineRuntime/AssetSubsystem.h"
#include "VGEngineRuntime/SceneSubsystem.h"
#include "VGEngineRuntime/TimingSystem.h"

namespace visiongal::engine
{
/**
 * @brief 行程級引擎 Runtime Facade（Phase 4）：**Initialize → Tick → Shutdown** 由宿主單執行緒驅動。
 *
 * 契約：
 * - `Tick` 僅允許在與 `Initialize` / `Shutdown` 相同之控制執行緒呼叫（測試與未來 game loop）。
 * - `Shutdown` 會阻塞直至 **AsyncSystem** 背景工作 join 完成；請勿於 Async 回呼內呼叫 `Shutdown`。
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

	bool IsInitialized() const noexcept { return initialized_; }

private:
	VGEngineRuntime() noexcept = default;

	bool initialized_{false};
	TimingSystem timing_{};
	AsyncSystem async_{};
	SceneSubsystem scene_{};
	AssetSubsystem asset_{};
};
} // namespace visiongal::engine
