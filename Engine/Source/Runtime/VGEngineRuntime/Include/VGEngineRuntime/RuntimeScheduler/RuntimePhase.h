#pragma once

#include <cstdint>

namespace visiongal::engine
{
/**
 * @brief **RuntimePhase**（占位）：未来用于 **PlayerLoop** 式可配置阶段 ID，与 **RuntimeTickGroup** 解耦或细分子阶段。
 *
 * 首包管线顺序硬编码于 **RuntimeScheduler::Tick**；请勿在本头添加运行态逻辑。
 */
enum class RuntimePhase : std::uint8_t
{
	Unspecified = 0,
};
} // namespace visiongal::engine
