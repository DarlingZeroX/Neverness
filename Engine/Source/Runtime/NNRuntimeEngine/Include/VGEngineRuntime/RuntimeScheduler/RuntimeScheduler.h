#pragma once

#include <cstddef>
#include <cstdint>

#include "NNRuntimeEngine/RuntimeScheduler/RuntimeFrameContext.h"
#include "NNRuntimeEngine/RuntimeScheduler/RuntimeSubsystem.h"
#include "NNRuntimeEngine/RuntimeScheduler/RuntimeSubsystemCollection.h"

namespace NN::Runtime::engine
{
/**
 * @brief **VGRuntimeScheduler** 首包实现：统一 **RuntimeTickGroup** 顺序、**FixedUpdate** 累加、**LateUpdate** 末 **FlushMainThreadDelegates** 占位。
 *
 * **FixedUpdate**：`fixedAccumulator_ += context.deltaTimeSeconds`，当 `>= fixedDeltaTimeSeconds_` 时循环调用 **FixedUpdate** 桶（每帧最多 **kMaxFixedStepsPerFrame** 次以防螺旋）。
 */
class RuntimeScheduler final
{
public:
	/** @brief 默认固定步长（50 Hz），与首包文档示例一致。 */
	static constexpr float kDefaultFixedDeltaTimeSeconds = 1.f / 50.f;
	static constexpr unsigned kMaxFixedStepsPerFrame = 5u;

	RuntimeScheduler() noexcept = default;

	void RegisterSubsystem(IRuntimeSubsystem* subsystem) noexcept;
	[[nodiscard]] bool UnregisterSubsystem(const IRuntimeSubsystem* subsystem) noexcept;

	void InitializeRegistered() noexcept;
	void ShutdownRegistered() noexcept;

	void Tick(const RuntimeFrameContext& context) noexcept;

	void SetFixedDeltaTimeSeconds(float seconds) noexcept { fixedDeltaTimeSeconds_ = seconds; }
	[[nodiscard]] float GetFixedDeltaTimeSeconds() const noexcept { return fixedDeltaTimeSeconds_; }

private:
	void TickBucket(RuntimeTickGroup group, const RuntimeFrameContext& context) noexcept;
	void RunFixedPasses(const RuntimeFrameContext& context) noexcept;
	void FlushMainThreadDelegates() noexcept;

	RuntimeSubsystemCollection collection_{};
	float fixedAccumulator_{0.f};
	float fixedDeltaTimeSeconds_{kDefaultFixedDeltaTimeSeconds};
};
} // namespace visiongal::engine
