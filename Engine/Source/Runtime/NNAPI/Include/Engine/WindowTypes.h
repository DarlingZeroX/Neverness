#pragma once

/**
 * @file WindowTypes.h
 * @brief 窗口子系统不透明句柄（**NNWindowHandle**）。
 *
 * **设计原则**
 * - 句柄为单调递增的 **uint64** 整数，**禁止**将 `SDL_Window*` 或其它原生指针 `reinterpret_cast` 为句柄。
 * - 数值 **0** 表示无效；非零不保证窗口仍存活（销毁后须视为无效）。
 * - 与 Vulkan / DX12 / Metal / RmlUi 等互操作时，请使用 `NNWindowAPI::getNativeHandle` 获取平台原生窗口指针。
 */

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief 不透明窗口控制码；由 Runtime **WindowRegistry** 分配，不暴露 SDL 类型。 */
typedef std::uint64_t NNWindowHandle;

/** @brief 无效窗口句柄常量。 */
static constexpr NNWindowHandle NN_INVALID_WINDOW_HANDLE = 0;

#ifdef __cplusplus
} /* extern "C" */
#endif
