#pragma once

#include <cstdint>

namespace visiongal::engine
{
/**
 * @brief 单帧只读上下文：由 **VGEngineRuntime::Tick** 在 **TimingSystem::Tick** 之后构造，传入各 **IRuntimeSubsystem::Tick**。
 *
 * **线程**：与 **VGEngineRuntime** 控制线程相同（单线程 game loop 假设）。
 */
struct RuntimeFrameContext final
{
	float deltaTimeSeconds{0.f};
	float totalTimeSeconds{0.f};
	std::uint64_t frameIndex{0};
	/** @brief 固定步长（秒）；用于 **FixedUpdate** 子系统与累加器裁切。 */
	float fixedDeltaTimeSeconds{0.f};
};
} // namespace visiongal::engine
