#include "AsyncSystem.h"

#include <chrono>
#include <thread>

namespace NN::Runtime::engine
{
struct AsyncSystem::Slot
{
	std::thread worker{};
	std::atomic<bool> done{false};
};

AsyncSystem::AsyncSystem() noexcept = default;

AsyncSystem::~AsyncSystem()
{
	Shutdown();
}

std::uint64_t AsyncSystem::CreateWait()
{
	std::lock_guard<std::mutex> lock(mutex_);
	if (shuttingDown_)
	{
		return 0;
	}

	const std::uint64_t id = nextHandle_.fetch_add(1u, std::memory_order_relaxed);
	auto slot = std::make_unique<Slot>();
	std::atomic<bool>* doneFlag = &slot->done;
	slot->worker = std::thread([doneFlag] {
		std::this_thread::sleep_for(std::chrono::milliseconds(12));
		doneFlag->store(true, std::memory_order_release);
	});
	slots_.emplace(id, std::move(slot));
	return id;
}

int AsyncSystem::TryComplete(std::uint64_t handle) noexcept
{
	if (handle == 0)
	{
		return 0;
	}

	std::lock_guard<std::mutex> lock(mutex_);
	const auto it = slots_.find(handle);
	if (it == slots_.end())
	{
		return 0;
	}
	return it->second->done.load(std::memory_order_acquire) ? 1 : 0;
}

void AsyncSystem::ReleaseWait(std::uint64_t handle) noexcept
{
	if (handle == 0)
	{
		return;
	}

	std::unique_ptr<Slot> slot;
	{
		std::lock_guard<std::mutex> lock(mutex_);
		const auto it = slots_.find(handle);
		if (it == slots_.end())
		{
			return;
		}
		slot = std::move(it->second);
		slots_.erase(it);
	}

	if (slot->worker.joinable())
	{
		slot->worker.join();
	}
}

void AsyncSystem::Shutdown() noexcept
{
	std::unordered_map<std::uint64_t, std::unique_ptr<Slot>> copy;
	{
		std::lock_guard<std::mutex> lock(mutex_);
		shuttingDown_ = true;
		copy.swap(slots_);
	}

	for (auto& p : copy)
	{
		if (p.second && p.second->worker.joinable())
		{
			p.second->worker.join();
		}
	}

	std::lock_guard<std::mutex> clearShutdownFlag(mutex_);
	shuttingDown_ = false;
}
} // namespace visiongal::engine
