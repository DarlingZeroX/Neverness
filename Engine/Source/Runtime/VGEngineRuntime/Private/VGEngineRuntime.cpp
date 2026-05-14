#include "VGEngineRuntime/VGEngineRuntime.h"

namespace visiongal::engine
{
VGEngineRuntime& VGEngineRuntime::Instance() noexcept
{
	static VGEngineRuntime s_instance;
	return s_instance;
}

bool VGEngineRuntime::Initialize() noexcept
{
	if (initialized_)
	{
		return true;
	}

	timing_.Reset();
	initialized_ = true;
	return true;
}

void VGEngineRuntime::Tick(float deltaTimeSeconds) noexcept
{
	if (!initialized_)
	{
		return;
	}

	timing_.Tick(deltaTimeSeconds);
}

void VGEngineRuntime::Shutdown() noexcept
{
	if (!initialized_)
	{
		return;
	}

	async_.Shutdown();
	initialized_ = false;
}
} // namespace visiongal::engine
