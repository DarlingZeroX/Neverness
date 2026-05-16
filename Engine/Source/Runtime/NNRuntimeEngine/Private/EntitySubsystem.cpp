/**
 * @file EntitySubsystem.cpp
 * @brief **EntitySubsystem** 实现：为 **VGEntityAPI::getRuntimeTick** 提供单调计数，为 **getServiceAbiToken** 提供与 Stub 一致的魔数。
 *
 * **与 `VGEngineRuntime::Tick` 的契约**
 * - 经 **`RuntimeScheduler`** 之 **`RuntimeTickGroup::Update`** 调用 **`IRuntimeSubsystem::Tick`** → **`OnTick`**（宿主单控制线程），故 **`runtimeTick_`** 使用 **`memory_order_relaxed`** 即可满足与托管侧「可观测」语义。
 * - **`deltaTimeSeconds`** 当前未参与计数；若未来与 **TimingSystem** 帧序号对齐，可在此读取 **Timing** 而非自增，但须另行评审 ABI 文档。
 */

#include "NNRuntimeEngine/EntitySubsystem.h"

#include "NNRuntimeEngine/RuntimeScheduler/RuntimeFrameContext.h"

namespace visiongal::engine
{
void EntitySubsystem::Tick(const RuntimeFrameContext& context) noexcept
{
	OnTick(context.deltaTimeSeconds);
}

RuntimeTickGroup EntitySubsystem::TickGroup() const noexcept
{
	return RuntimeTickGroup::Update;
}

void EntitySubsystem::Shutdown() noexcept
{
	Reset();
}

void EntitySubsystem::OnTick(float deltaTimeSeconds) noexcept
{
	(void)deltaTimeSeconds;
	// 单调帧计数：与 Timing 子系统解耦，仅表示 Entity 子系统已被 Tick 驱动（供 getRuntimeTick ABI 观测）。
	runtimeTick_.fetch_add(1u, std::memory_order_relaxed);
}

void EntitySubsystem::Reset() noexcept
{
	runtimeTick_.store(0u, std::memory_order_relaxed);
}

std::uint32_t EntitySubsystem::GetServiceAbiToken() const noexcept
{
	return VG_ENTITY_SERVICE_ABI_TOKEN;
}

std::uint64_t EntitySubsystem::GetRuntimeTick() const noexcept
{
	return runtimeTick_.load(std::memory_order_relaxed);
}
} // namespace visiongal::engine
