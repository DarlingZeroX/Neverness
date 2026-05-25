#pragma once

/**
 * @file EventAPI.h
 * @brief ABI-stable 事件函数表（NNEventAPI）。
 *
 * Pull-Based Event Queue Architecture：
 * - C# 每帧调用 pollEvent 批量消费事件
 * - 字符串（DROP_FILE 路径等）通过 String Pool 传递
 * - 无 callback、无 GC pinning、无 std::string
 *
 * 集成到 NNNativeEngineAPI 作为 'events' 子表。
 */

#include "EventTypes.h"
#include "NativeInterop.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * NNEventAPI — 事件函数表
 * ======================================================================== */

typedef struct NNEventAPI
{
    /** sizeof(NNEventAPI)，用于前向兼容。 */
    std::uint32_t size;

    /**
     * @brief 从队列消费一个事件。
     * @param outEvent [out] 接收事件数据（128 字节拷贝）。
     * @return 1 = 已出队，0 = 队列空。
     */
    std::uint32_t (NN_ENGINE_ABI_STDCALL * pollEvent)(
        NNEvent* outEvent);

    /**
     * @brief 窥视下一个事件但不消费。
     * @param outEvent [out] 接收事件副本。
     * @return 1 = 有事件，0 = 队列空。
     */
    std::uint32_t (NN_ENGINE_ABI_STDCALL * peekEvent)(
        NNEvent* outEvent);

    /**
     * @brief 阻塞等待事件（带超时）。
     * @param outEvent [out] 接收事件。
     * @param timeoutMs 最大等待毫秒数（0 = 非阻塞 poll）。
     * @return 1 = 已出队，0 = 超时。
     */
    std::uint32_t (NN_ENGINE_ABI_STDCALL * waitEvent)(
        NNEvent* outEvent,
        std::uint32_t timeoutMs);

    /**
     * @brief 从事件的 stringPoolIdx 读取 UTF-8 字符串。
     * @param event [in] 包含 stringPoolIdx 的事件。
     * @param outPtr [out] UTF-8 字符串指针（下次 pollEvent 前有效）。
     * @param outLen [out] 字节长度。
     * @return 1 = 找到字符串，0 = 无效索引。
     */
    std::uint32_t (NN_ENGINE_ABI_STDCALL * getEventString)(
        const NNEvent* event,
        const char** outPtr,
        std::uint16_t* outLen);

    /**
     * @brief 获取队列中当前事件数量。
     */
    std::uint32_t (NN_ENGINE_ABI_STDCALL * getQueueCount)(void);

    /**
     * @brief 清空事件队列（如窗口失去焦点时）。
     */
    void (NN_ENGINE_ABI_STDCALL * flushEvents)(void);

    /**
     * @brief 推送用户自定义事件（C# → Native）。
     * @return 1 = 成功，0 = 队列满。
     */
    std::uint32_t (NN_ENGINE_ABI_STDCALL * pushUserEvent)(
        const NNEvent* event);

} NNEventAPI;

#ifdef __cplusplus
}
#endif
