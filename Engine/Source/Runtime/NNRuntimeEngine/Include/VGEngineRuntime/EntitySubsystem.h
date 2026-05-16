#pragma once

#include <atomic>
#include <cstdint>

#include "NNNativeEngineAPI/EntityAPI.h"
#include "NNRuntimeEngine/RuntimeScheduler/RuntimeSubsystem.h"

namespace NN::Runtime::engine
{
struct RuntimeFrameContext;

/**
 * @brief 行程级 **Entity** 子系统首包（对应路线图 **VGEntitySystem** 在 **VGEngineRuntime** 内的承载体）。
 *
 * **职责（本切片）**
 * - 实现 **IRuntimeSubsystem**，由 **RuntimeScheduler** 在 **RuntimeTickGroup::Update** 阶段驱动（**P0-1**）。
 * - 维护 **`runtimeTick`**：每帧 **`Tick`** 时单调递增，供 **`VGEntityAPI::getRuntimeTick`**
 *   观测「Runtime 表已覆写 **`entity.*`**」。
 * - **`GetServiceAbiToken`**：返回 **`VG_ENTITY_SERVICE_ABI_TOKEN`**，与 Stub 及托管常量一致，用于 ABI 冒烟不断裂。
 *
 * **边界**
 * - **不**实现完整 Native ECS，**不**与托管 **EntityWorld** 做数据结构同步；与 **`VGSceneAPI`** 之 **`VGEntityHandle`** 语义仍独立（见 **`EntityAPI.h`**）。
 */
class EntitySubsystem final : public IRuntimeSubsystem
{
public:
	void Tick(const RuntimeFrameContext& context) noexcept override;
	[[nodiscard]] RuntimeTickGroup TickGroup() const noexcept override;
	void Shutdown() noexcept override;

	/** @brief 兼容旧调用点；等价于 **`Tick`** 内对 **`deltaTimeSeconds`** 的处理。 */
	void OnTick(float deltaTimeSeconds) noexcept;
	void Reset() noexcept;

	[[nodiscard]] std::uint32_t GetServiceAbiToken() const noexcept;
	[[nodiscard]] std::uint64_t GetRuntimeTick() const noexcept;

private:
	std::atomic<std::uint64_t> runtimeTick_{0};
};
} // namespace visiongal::engine
