#pragma once

/**
 * @file WindowAPI.h
 * @brief **NNWindowAPI**：工业级 Runtime 窗口子系统 C ABI（多窗口、Native 句柄、无 SDL 泄漏）。
 *
 * **职责边界**
 * - 负责：窗口创建/销毁、标题、尺寸、位置、可见性、最大化/最小化、跨平台 **Native** 句柄（HWND / NSWindow / X11）。
 * - 不负责：SDL 子系统生命周期、全局事件泵、帧 Begin/End（见 `ApplicationAPI.h`）。
 *
 * **扩展规则**
 * - 仅允许在 `NNWindowAPI` 结构体 **尾部追加** 函数指针；破坏性变更须递增 `NN_NATIVE_ENGINE_API_LAYOUT_VERSION`。
 * - **禁止** 在任何 ABI 头文件中暴露 `SDL_Window*`。
 */

#include <cstdint>

#include "NativeInterop.h"
#include "WindowTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 创建窗口时的描述块（POD，可 blittable 至 C# / NativeAOT）。
 */
typedef struct NNWindowDesc
{
	/** @brief UTF-8 标题；可为 nullptr，实现方回退默认标题。 */
	const char* title;
	int width;
	int height;
	/** @brief 是否允许用户调整客户区尺寸。 */
	bool resizable;
	/** @brief 创建后是否最大化。 */
	bool maximized;
	/** @brief 创建后是否隐藏（不显示于任务栏可见状态前可先 hidden）。 */
	bool hidden;
} NNWindowDesc;

typedef NNWindowHandle(NN_ENGINE_ABI_STDCALL* NNWindowCreateFn)(const NNWindowDesc* desc);

typedef void(NN_ENGINE_ABI_STDCALL* NNWindowDestroyFn)(NNWindowHandle handle);

typedef void(NN_ENGINE_ABI_STDCALL* NNWindowSetTitleFn)(NNWindowHandle handle, const char* title);

typedef void(NN_ENGINE_ABI_STDCALL* NNWindowSetSizeFn)(NNWindowHandle handle, int width, int height);

typedef void(NN_ENGINE_ABI_STDCALL* NNWindowGetSizeFn)(NNWindowHandle handle, int* outWidth, int* outHeight);

typedef void(NN_ENGINE_ABI_STDCALL* NNWindowSetPositionFn)(NNWindowHandle handle, int x, int y);

typedef void(NN_ENGINE_ABI_STDCALL* NNWindowGetPositionFn)(NNWindowHandle handle, int* outX, int* outY);

typedef void(NN_ENGINE_ABI_STDCALL* NNWindowSetResizableFn)(NNWindowHandle handle, bool value);

typedef void(NN_ENGINE_ABI_STDCALL* NNWindowMaximizeFn)(NNWindowHandle handle);

typedef void(NN_ENGINE_ABI_STDCALL* NNWindowMinimizeFn)(NNWindowHandle handle);

typedef void(NN_ENGINE_ABI_STDCALL* NNWindowRestoreFn)(NNWindowHandle handle);

typedef void(NN_ENGINE_ABI_STDCALL* NNWindowShowFn)(NNWindowHandle handle);

typedef void(NN_ENGINE_ABI_STDCALL* NNWindowHideFn)(NNWindowHandle handle);

/**
 * @brief 获取平台原生窗口指针（**非** SDL_Window*）。
 * - Win32: HWND
 * - macOS: NSWindow*
 * - Linux: X11 Window（具体类型见 SDL3 文档）
 */
typedef void*(NN_ENGINE_ABI_STDCALL* NNWindowGetNativeHandleFn)(NNWindowHandle handle);

typedef struct NNWindowAPI
{
	std::uint32_t size;

	NNWindowCreateFn create;
	NNWindowDestroyFn destroy;

	NNWindowSetTitleFn setTitle;

	NNWindowSetSizeFn setSize;
	NNWindowGetSizeFn getSize;

	NNWindowSetPositionFn setPosition;
	NNWindowGetPositionFn getPosition;

	NNWindowSetResizableFn setResizable;

	NNWindowMaximizeFn maximize;
	NNWindowMinimizeFn minimize;
	NNWindowRestoreFn restore;

	NNWindowShowFn show;
	NNWindowHideFn hide;

	NNWindowGetNativeHandleFn getNativeHandle;
} NNWindowAPI;

#ifdef __cplusplus
} /* extern "C" */
#endif
