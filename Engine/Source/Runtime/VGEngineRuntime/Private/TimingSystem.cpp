#include "VGEngineRuntime/TimingSystem.h"

namespace visiongal::engine
{
void TimingSystem::Reset() noexcept
{
	lastDelta_ = 0.f;
	totalTime_ = 0.f;
	frameIndex_ = 0;
}

void TimingSystem::Tick(float deltaTimeSeconds) noexcept
{
	lastDelta_ = deltaTimeSeconds;
	totalTime_ += deltaTimeSeconds;
	++frameIndex_;
}
} // namespace visiongal::engine
