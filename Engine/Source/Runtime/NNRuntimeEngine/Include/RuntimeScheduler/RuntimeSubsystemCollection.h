#pragma once

#include <array>
#include <cstddef>
#include <vector>

#include "RuntimeScheduler/RuntimeSubsystem.h"
#include "RuntimeScheduler/RuntimeTickGroup.h"

namespace NN::Runtime::engine
{
/**
 * @brief 按 **RuntimeTickGroup** 分桶保存 **IRuntimeSubsystem***，同桶内保持 **注册顺序**（稳定排序）。
 */
class RuntimeSubsystemCollection final
{
public:
	void Register(IRuntimeSubsystem* subsystem) noexcept;
	[[nodiscard]] bool Unregister(const IRuntimeSubsystem* subsystem) noexcept;

	void Clear() noexcept;

	[[nodiscard]] const std::vector<IRuntimeSubsystem*>& Bucket(RuntimeTickGroup group) const noexcept;

private:
	static std::size_t Index(RuntimeTickGroup group) noexcept;

	std::array<std::vector<IRuntimeSubsystem*>, RuntimeTickGroupCount> buckets_{};
};
} // namespace visiongal::engine
