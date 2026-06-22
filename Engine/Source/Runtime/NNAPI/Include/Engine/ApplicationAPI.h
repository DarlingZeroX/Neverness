#pragma once

/**
 * @file ApplicationAPI.h
 * @brief **NNApplicationAPI**：Runtime Host 生命周期层（SDL 子系统、事件泵、帧边界）。
 *
 * **职责边界**
 * - 负责：`SDL_Init` / `SDL_Quit`（由实现方在 initialize/shutdown 内完成）、全局 `PollEvent`、帧 `beginFrame` / `endFrame` 扩展点。
 * - 不负责：窗口创建、标题、尺寸、Native 句柄（见 `WindowAPI.h`）；Gameplay、Scene、ECS、Editor UI。
 *
 * **扩展规则**
 * - 仅允许在结构体 **尾部追加** 函数指针；破坏性变更须递增 `NN_NATIVE_ENGINE_API_LAYOUT_VERSION`。
 */

#include <cstdint>

#include "NativeInterop.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief 初始化 SDL 子系统；可幂等，已初始化则返回 1，失败返回 0。 */
typedef int(NN_ENGINE_ABI_STDCALL* NNApplicationInitializeFn)(void);

/** @brief 泵送事件；返回 false 表示应退出主循环（例如收到 Quit）。 */
typedef bool(NN_ENGINE_ABI_STDCALL* NNApplicationPumpEventsFn)(void);

/** @brief 关闭所有窗口并退出 SDL（实现方应销毁 WindowRegistry 内窗口）。 */
typedef void(NN_ENGINE_ABI_STDCALL* NNApplicationShutdownFn)(void);

/** @brief 帧开始钩子（ImGui / 渲染同步点等由实现方挂接）。 */
typedef void(NN_ENGINE_ABI_STDCALL* NNApplicationBeginFrameFn)(void);

/** @brief 帧结束钩子（交换缓冲区等）。 */
typedef void(NN_ENGINE_ABI_STDCALL* NNApplicationEndFrameFn)(void);

typedef struct NNApplicationAPI
{
	std::uint32_t size;
	NNApplicationInitializeFn initialize;
	NNApplicationPumpEventsFn pumpEvents;
	NNApplicationShutdownFn shutdown;
	NNApplicationBeginFrameFn beginFrame;
	NNApplicationEndFrameFn endFrame;
} NNApplicationAPI;

#ifdef __cplusplus
} /* extern "C" */
#endif
