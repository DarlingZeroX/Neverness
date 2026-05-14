#include <atomic>
#include <cstdio>
#include <mutex>

#include "VGManagedCore/NativeAPI.h"

namespace
{
	// 诊断计数：验证托管 → Native 经函数指针调用是否真正到达实现（GTest 使用）。
	std::atomic<std::uint32_t> g_logInfoCallCount{0};
	std::mutex g_logMutex;
} // namespace

extern "C" void VG_NATIVE_STDCALL VGNativeApi_DefaultLogInfo(const char* messageUtf8)
{
	// 防御：托管错误或未初始化指针可能传入 null，避免 native 崩溃。
	if (messageUtf8 == nullptr)
	{
		return;
	}

	g_logInfoCallCount.fetch_add(1, std::memory_order_relaxed);

	// Phase 2：最小可观测行为 — 写入 stderr（UTF-8 字节序列，无宽字符转换）。
	std::lock_guard<std::mutex> lock(g_logMutex);
	std::fputs(messageUtf8, stderr);
	std::fputc('\n', stderr);
	std::fflush(stderr);
}

extern "C" std::uint32_t VGNativeApi_GetLogInfoCallCount(void)
{
	return g_logInfoCallCount.load(std::memory_order_relaxed);
}
