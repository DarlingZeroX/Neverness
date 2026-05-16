#include "NNRuntimeEngine/RuntimeScheduler/RuntimeSubsystemCollection.h"

#include <algorithm>

namespace NN::Runtime::engine
{
void RuntimeSubsystemCollection::Register(IRuntimeSubsystem* subsystem) noexcept
{
	if (subsystem == nullptr)
	{
		return;
	}
	std::vector<IRuntimeSubsystem*>& bucket = buckets_[Index(subsystem->TickGroup())];
	if (std::find(bucket.begin(), bucket.end(), subsystem) != bucket.end())
	{
		return;
	}
	bucket.push_back(subsystem);
}

bool RuntimeSubsystemCollection::Unregister(const IRuntimeSubsystem* subsystem) noexcept
{
	if (subsystem == nullptr)
	{
		return false;
	}
	const RuntimeTickGroup group = subsystem->TickGroup();
	auto& bucket = buckets_[Index(group)];
	const auto it = std::find(bucket.begin(), bucket.end(), subsystem);
	if (it == bucket.end())
	{
		return false;
	}
	bucket.erase(it);
	return true;
}

void RuntimeSubsystemCollection::Clear() noexcept
{
	for (auto& bucket : buckets_)
	{
		bucket.clear();
	}
}

const std::vector<IRuntimeSubsystem*>& RuntimeSubsystemCollection::Bucket(RuntimeTickGroup group) const noexcept
{
	return buckets_[Index(group)];
}

std::size_t RuntimeSubsystemCollection::Index(RuntimeTickGroup group) noexcept
{
	return static_cast<std::size_t>(group);
}
} // namespace visiongal::engine
