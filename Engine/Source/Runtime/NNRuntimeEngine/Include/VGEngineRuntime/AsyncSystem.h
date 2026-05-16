#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace NN::Runtime::engine
{
/**
 * @brief 可輪詢完成之非同步等待（背景 `std::thread` + `std::mutex`）。
 *
 * `tryComplete`：未完成回傳 0；完成後回傳 1（之後仍回傳 1，直到 `releaseWait`）。
 * `Shutdown`：join 所有尚未釋放之等待；與 `createWait` 互斥。
 */
class AsyncSystem final
{
public:
	AsyncSystem() noexcept;
	~AsyncSystem();

	AsyncSystem(const AsyncSystem&) = delete;
	AsyncSystem& operator=(const AsyncSystem&) = delete;
	AsyncSystem(AsyncSystem&&) noexcept = delete;
	AsyncSystem& operator=(AsyncSystem&&) noexcept = delete;

	std::uint64_t CreateWait();
	int TryComplete(std::uint64_t handle) noexcept;
	void ReleaseWait(std::uint64_t handle) noexcept;

	void Shutdown() noexcept;

private:
	struct Slot;

	std::mutex mutex_{};
	std::unordered_map<std::uint64_t, std::unique_ptr<Slot>> slots_{};
	std::atomic<std::uint64_t> nextHandle_{1};
	bool shuttingDown_{false};
};
} // namespace visiongal::engine
