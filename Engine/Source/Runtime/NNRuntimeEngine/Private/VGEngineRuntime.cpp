/**
 * @file VGEngineRuntime.cpp
 * @brief **VGEngineRuntime** 单例：`Initialize` / `Tick` / `Shutdown` 与 **RuntimeScheduler** / **EntitySubsystem** 的驱动顺序见下。
 *
 * **`Tick` 顺序（P0-1+）**
 * - 先 **`timing_.Tick`**，再构造 **RuntimeFrameContext**，最后 **`scheduler_.Tick`**（其中 **EntitySubsystem** 位于 **RuntimeTickGroup::Update**），以便与现有 Phase 4 测试（帧序号、累计时间）保持一致；**`getRuntimeTick`** 表示 Entity 子系统已被推进，与 **Timing** 的 **frameIndex** 无强制数值相等关系。
 *
 * **`Shutdown`**
 * - 先 **`async_.Shutdown`**（join 后台等待），再 **`scheduler_.ShutdownRegistered()`**（**EntitySubsystem::Shutdown** 内 **`Reset`**）将 **`runtimeTick`** 清零，便于下次 **`Initialize`** 后从 0 重新观测；与 MANAGED **§2.7.1** 文档一句说明一致。
 */

#include "VGEngineRuntime.h"

#include "RuntimeScheduler/RuntimeFrameContext.h"

namespace NN::Runtime::engine
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
	scheduler_.RegisterSubsystem(&entity_);
	sceneTick_.SetScene(&ecsScene_);
	scheduler_.RegisterSubsystem(&sceneTick_);
	scheduler_.InitializeRegistered();
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
	RuntimeFrameContext frame{};
	frame.deltaTimeSeconds = timing_.GetDeltaTime();
	frame.totalTimeSeconds = timing_.GetTotalTime();
	frame.frameIndex = timing_.GetFrameIndex();
	frame.fixedDeltaTimeSeconds = scheduler_.GetFixedDeltaTimeSeconds();
	scheduler_.Tick(frame);
}

void VGEngineRuntime::Shutdown() noexcept
{
	if (!initialized_)
	{
		return;
	}

	async_.Shutdown();
	scheduler_.ShutdownRegistered();
	initialized_ = false;
}
} // namespace NN::Runtime::engine
