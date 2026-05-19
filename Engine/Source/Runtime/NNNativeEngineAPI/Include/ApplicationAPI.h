#pragma once

/**
 * @file ApplicationAPI.h
 * @brief **NNApplicationAPI**：统一 Runtime Application 层（SDL 生命周期、主窗口、事件泵）。
 *
 * **职责边界**
 * - 负责：SDL_Init/Quit、主窗口、PollEvents、后续 Frame Begin/End 扩展点。
 * - 不负责：Gameplay、Scene Runtime、RuntimeLoop、ECS、Editor UI（均由 Managed / Kernel 承担）。
 *
 * **扩展规则**
 * - 仅允许在结构体 **尾部追加** 函数指针；破坏性变更须递增 `NN_NATIVE_ENGINE_API_LAYOUT_VERSION`。
 */

#include <cstdint>

#include "NativeInterop.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef bool(NN_ENGINE_ABI_STDCALL* NNApplicationInitializeFn)(void);

typedef bool(NN_ENGINE_ABI_STDCALL* NNApplicationOpenWindowFn)(const char* title, int width, int height);

/** @brief 泵送事件；返回 false 表示应退出主循环（例如收到 Quit）。 */
typedef bool(NN_ENGINE_ABI_STDCALL* NNApplicationPumpEventsFn)(void);

typedef void(NN_ENGINE_ABI_STDCALL* NNApplicationShutdownFn)(void);

typedef struct NNApplicationAPI
{
	std::uint32_t size;
	NNApplicationInitializeFn initialize;
	NNApplicationOpenWindowFn openWindow;
	NNApplicationPumpEventsFn pumpEvents;
	NNApplicationShutdownFn shutdown;
} NNApplicationAPI;

#ifdef __cplusplus
} /* extern "C" */
#endif
