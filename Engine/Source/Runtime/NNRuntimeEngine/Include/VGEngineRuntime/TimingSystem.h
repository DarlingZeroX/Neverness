#pragma once

#include <cstdint>

namespace visiongal::engine
{
/**
 * @brief 以 `Tick` 累積之時間狀態；**getFrameIndex** 於每次 `Tick` 結束後遞增（首次 `Tick` 後為 1）。
 */
class TimingSystem final
{
public:
	void Reset() noexcept;
	void Tick(float deltaTimeSeconds) noexcept;

	float GetDeltaTime() const noexcept { return lastDelta_; }
	float GetTotalTime() const noexcept { return totalTime_; }
	std::uint64_t GetFrameIndex() const noexcept { return frameIndex_; }

private:
	float lastDelta_{0.f};
	float totalTime_{0.f};
	std::uint64_t frameIndex_{0};
};
} // namespace visiongal::engine
