#pragma once

#include "RuntimeScheduler/RuntimeFrameContext.h"
#include "RuntimeScheduler/RuntimeTickGroup.h"

namespace NN::Runtime::engine
{
/**
 * @brief 可注册至 **RuntimeScheduler** 的运行时子系统接口（**P0-1 Kernel** 统一 Tick 管线）。
 *
 * **生命周期**（与 **VGEngineRuntime** 一致）：
 * - **Initialize**：在宿主 **Initialize** 阶段、**RegisterSubsystem** 之后由调度器统一调用一次。
 * - **Tick**：每帧在对应 **RuntimeTickGroup** 阶段调用；**不得**在 **Tick** 内阻塞 join **AsyncSystem**（见 **VGEngineRuntime** 契约）。
 * - **Shutdown**：宿主 **Shutdown** 时逆序调用；指针须存活至 **UnregisterSubsystem** 或 **ShutdownRegistered** 完成。
 *
 * **注意**：当前仅 **NNRuntimeSceneTickSubsystem** 实现本接口；其余子系统仍由 Facade 直接持有。
 */
class IRuntimeSubsystem
{
public:
	virtual ~IRuntimeSubsystem() = default;

	virtual void Initialize() noexcept {}
	virtual void Shutdown() noexcept {}

	virtual void Tick(const RuntimeFrameContext& context) noexcept = 0;

	[[nodiscard]] virtual RuntimeTickGroup TickGroup() const noexcept = 0;
};
} // namespace visiongal::engine
