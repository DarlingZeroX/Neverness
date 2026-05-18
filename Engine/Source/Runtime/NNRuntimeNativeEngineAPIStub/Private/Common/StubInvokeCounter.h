#pragma once

/**
 * @file StubInvokeCounter.h
 * @brief Stub Runtime 跨子系統呼叫計數（供 **VGManagedHostTest** 等斷言 ABI 鏈路已觸達）。
 */

#include <cstdint>

namespace NN::StubRuntime
{
/** @brief 將全域 Stub 呼叫計數加一（記憶體序為 relaxed，僅用於測試觀測）。 */
void BumpInvokeCount() noexcept;
} // namespace NN::StubRuntime
