/**
 * @file StubInvokeCounter.cpp
 * @brief Stub 呼叫計數實作與 **NNNativeEngineApi_GetStubInvokeCount** 匯出。
 */

#include "Common/StubInvokeCounter.h"

#include <atomic>

namespace
{
	std::atomic<std::uint32_t> g_stubInvokeCount{0};
} // namespace

namespace NN::StubRuntime
{
void BumpInvokeCount() noexcept
{
	g_stubInvokeCount.fetch_add(1u, std::memory_order_relaxed);
}
} // namespace NN::StubRuntime

extern "C" std::uint32_t NNNativeEngineApi_GetStubInvokeCount(void)
{
	return g_stubInvokeCount.load(std::memory_order_relaxed);
}
