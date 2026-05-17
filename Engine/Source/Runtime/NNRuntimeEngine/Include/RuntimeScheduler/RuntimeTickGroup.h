#pragma once

#include <cstddef>

namespace NN::Runtime::engine
{
/**
 * @brief Runtime **Tick 分组**（**VGRuntimeScheduler** / **RuntimeScheduler** 管线顺序的语义锚点）。
 *
 * 顺序概览（每帧）：**EarlyUpdate** →（0..N 次 **FixedUpdate**）→ **Update** → **LateUpdate** → **Render**。
 * 与 Unity **PlayerLoop** / UE **TickingGroup** 概念对齐，首包仅 **EntitySubsystem** 挂在 **Update**。
 */
enum class RuntimeTickGroup : std::uint8_t
{
	EarlyUpdate = 0,
	FixedUpdate,
	Update,
	LateUpdate,
	Render,
};

/** @brief 分组数量，供 **RuntimeSubsystemCollection** 分桶使用。 */
inline constexpr std::size_t RuntimeTickGroupCount = 5u;
} // namespace visiongal::engine
