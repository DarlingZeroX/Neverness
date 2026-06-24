#pragma once

/**
 * @file EventQueue.h
 * @brief Lock-free SPSC Event Ring Buffer + String Pool。
 *
 * 设计：
 * - Single Producer（SDL 主线程写入）+ Single Consumer（C# 主线程读取）
 * - 同一线程既生产又消费 → 无需 mutex，原子操作即可
 * - 事件队列 4096 slot × 128B = 512KB
 * - String Pool 64KB，存储 DropFile 路径等 UTF-8 字符串
 *
 * String Pool 格式：[uint16_t length][...UTF-8 bytes...]
 * 通过 NNEvent.stringPoolIdx（字节偏移）引用。
 */

#include "Engine/EventTypes.h"

#include <atomic>
#include <cstring>

namespace NN::Runtime
{

/**
 * @brief SPSC 事件环形队列 + String Pool。
 *
 * 线程安全：
 * - Push() 仅由生产者（SDL 线程）调用
 * - Pop()  仅由消费者（主线程 C#）调用
 * - SPSC 模型：单生产者单消费者，原子操作无锁
 */
class EventQueue
{
public:
    /* 队列容量：必须是 2 的幂 */
    static constexpr std::uint32_t kCapacity  = 4096;
    static constexpr std::uint32_t kMask      = kCapacity - 1;

    /* String Pool 容量：64KB */
    static constexpr std::uint32_t kPoolSize  = 65536;

    EventQueue() = default;
	~EventQueue() = default;

    /* ================================================================
     * 生产者接口（SDL 线程调用）
     * ================================================================ */

    /**
     * @brief 推送事件到队列。
     * @return true = 入队成功，false = 队列满。
     */
    bool Push(const NNEvent& event) noexcept
    {
        const auto write = m_write.load(std::memory_order_relaxed);
        const auto nextWrite = (write + 1) & kMask;

        /* 保留一个 slot 作为空/满区分 */
        if (nextWrite == m_read.load(std::memory_order_acquire))
            return false;

        m_buffer[write] = event;
        m_write.store(nextWrite, std::memory_order_release);
        return true;
    }

    /**
     * @brief 推送事件，若队列尾部已有同类型同窗口的 Resize 则合并。
     * @return true = 已入队或已合并。
     */
    bool PushOrCoalesceResize(const NNEvent& event) noexcept
    {
        if (event.type != NN_EVENT_TYPE_WINDOW ||
            event.subtype != NN_WINDOW_RESIZED)
            return Push(event);

        /* 尝试合并：检查队列尾部 */
        const auto write = m_write.load(std::memory_order_relaxed);
        const auto read  = m_read.load(std::memory_order_acquire);

        if (write != read)
        {
            const auto prevIdx = (write - 1) & kMask;
            NNEvent& prev = m_buffer[prevIdx];

            if (prev.type == NN_EVENT_TYPE_WINDOW &&
                prev.subtype == NN_WINDOW_RESIZED &&
                prev.source == event.source)
            {
                prev = event; /* 覆盖旧 resize，保留最新尺寸 */
                return true;
            }
        }

        return Push(event);
    }

    /**
     * @brief 写入 UTF-8 字符串到 String Pool。
     * @return String Pool 字节偏移，UINT32_MAX = Pool 满。
     */
    std::uint32_t WriteString(const char* utf8, std::uint16_t len) noexcept
    {
        const std::uint32_t totalLen = 2u + len;

        auto pos = m_poolWrite.load(std::memory_order_relaxed);

        /* 回卷 */
        if (pos + totalLen > kPoolSize)
            pos = 0;

        /* 检查是否覆盖消费者读取位置 */
        const auto poolRead = m_poolRead.load(std::memory_order_acquire);
        if (pos < poolRead && pos + totalLen >= poolRead)
            return UINT32_MAX; /* Pool 满 */

        auto* dst = m_stringPool + pos;
        std::memcpy(dst, &len, 2);
        std::memcpy(dst + 2, utf8, len);

        const std::uint32_t result = pos;
        m_poolWrite.store(pos + totalLen, std::memory_order_release);
        return result;
    }

    /* ================================================================
     * 消费者接口（主线程 C# 调用）
     * ================================================================ */

    /**
     * @brief 从队列消费一个事件。
     * @return true = 已出队，false = 队列空。
     */
    bool Pop(NNEvent& out) noexcept
    {
        const auto read = m_read.load(std::memory_order_relaxed);

        if (read == m_write.load(std::memory_order_acquire))
            return false;

        out = m_buffer[read];
        m_read.store((read + 1) & kMask, std::memory_order_release);
        return true;
    }

    /**
     * @brief 窥视下一个事件但不消费。
     * @return true = 有事件。
     */
    bool Peek(NNEvent& out) const noexcept
    {
        const auto read = m_read.load(std::memory_order_relaxed);

        if (read == m_write.load(std::memory_order_acquire))
            return false;

        out = m_buffer[read];
        return true;
    }

    /**
     * @brief 从 String Pool 读取字符串。
     * @param idx 字节偏移（来自 NNEvent.stringPoolIdx）。
     * @param outPtr [out] 字符串指针。
     * @param outLen [out] 字节长度。
     * @return true = 有效。
     */
    bool ReadString(std::uint32_t idx, const char*& outPtr,
                    std::uint16_t& outLen) const noexcept
    {
        if (idx >= kPoolSize)
            return false;

        std::memcpy(&outLen, m_stringPool + idx, 2);
        outPtr = reinterpret_cast<const char*>(m_stringPool + idx + 2);
        return true;
    }

    /**
     * @brief 获取队列中当前事件数量（近似值，无锁）。
     */
    std::uint32_t Size() const noexcept
    {
        const auto w = m_write.load(std::memory_order_acquire);
        const auto r = m_read.load(std::memory_order_acquire);
        return (w - r) & kMask;
    }

    /**
     * @brief 清空事件队列。
     */
    void Clear() noexcept
    {
        m_read.store(
            m_write.load(std::memory_order_acquire),
            std::memory_order_release);
    }

private:
    /* 事件 Ring Buffer — 对齐到 cache line */
    alignas(64) NNEvent m_buffer[kCapacity]{};

    /* 生产者游标（SDL 线程写） */
    alignas(64) std::atomic<std::uint32_t> m_write{0};

    /* 消费者游标（主线程 C# 读） */
    alignas(64) std::atomic<std::uint32_t> m_read{0};

    /* String Pool */
    alignas(64) std::uint8_t m_stringPool[kPoolSize]{};

    /* String Pool 生产者游标 */
    alignas(64) std::atomic<std::uint32_t> m_poolWrite{0};

    /* String Pool 消费者游标 */
    alignas(64) std::atomic<std::uint32_t> m_poolRead{0};
};

} // namespace NN::Runtime
