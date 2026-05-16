/**
 * @file RuntimeScheduler.cpp
 * @brief **RuntimeScheduler** 实现：Early → Fixed（0..N）→ Update → Late（含主线程队列 Flush 占位）→ Render。
 */

#include "VGEngineRuntime/RuntimeScheduler/RuntimeScheduler.h"

namespace visiongal::engine
{
void RuntimeScheduler::RegisterSubsystem(IRuntimeSubsystem* subsystem) noexcept
{
	collection_.Register(subsystem);
}

bool RuntimeScheduler::UnregisterSubsystem(const IRuntimeSubsystem* subsystem) noexcept
{
	return collection_.Unregister(subsystem);
}

void RuntimeScheduler::InitializeRegistered() noexcept
{
	for (std::size_t gi = 0; gi < RuntimeTickGroupCount; ++gi)
	{
		for (IRuntimeSubsystem* const s : collection_.Bucket(static_cast<RuntimeTickGroup>(gi)))
		{
			if (s != nullptr)
			{
				s->Initialize();
			}
		}
	}
}

void RuntimeScheduler::ShutdownRegistered() noexcept
{
	for (std::size_t gi = RuntimeTickGroupCount; gi > 0u; --gi)
	{
		const std::size_t idx = gi - 1u;
		const std::vector<IRuntimeSubsystem*>& bucket = collection_.Bucket(static_cast<RuntimeTickGroup>(idx));
		for (auto it = bucket.rbegin(); it != bucket.rend(); ++it)
		{
			IRuntimeSubsystem* const s = *it;
			if (s != nullptr)
			{
				s->Shutdown();
			}
		}
	}
	fixedAccumulator_ = 0.f;
}

void RuntimeScheduler::Tick(const RuntimeFrameContext& context) noexcept
{
	fixedAccumulator_ += context.deltaTimeSeconds;

	TickBucket(RuntimeTickGroup::EarlyUpdate, context);
	RunFixedPasses(context);
	TickBucket(RuntimeTickGroup::Update, context);
	TickBucket(RuntimeTickGroup::LateUpdate, context);
	FlushMainThreadDelegates();
	TickBucket(RuntimeTickGroup::Render, context);
}

void RuntimeScheduler::TickBucket(RuntimeTickGroup group, const RuntimeFrameContext& context) noexcept
{
	for (IRuntimeSubsystem* const s : collection_.Bucket(group))
	{
		if (s != nullptr)
		{
			s->Tick(context);
		}
	}
}

void RuntimeScheduler::RunFixedPasses(const RuntimeFrameContext& context) noexcept
{
	const float step = fixedDeltaTimeSeconds_;
	if (step <= 0.f)
	{
		return;
	}
	unsigned iterations = 0u;
	while (fixedAccumulator_ >= step && iterations < kMaxFixedStepsPerFrame)
	{
		fixedAccumulator_ -= step;
		++iterations;
		RuntimeFrameContext fixedCtx = context;
		fixedCtx.deltaTimeSeconds = step;
		TickBucket(RuntimeTickGroup::FixedUpdate, fixedCtx);
	}
}

void RuntimeScheduler::FlushMainThreadDelegates() noexcept
{
	// 占位：未来将在此 drain 由 **AsyncSystem** 或其它后台工作排入的「完成回调 / 主线程任务队列」。
}
} // namespace visiongal::engine
