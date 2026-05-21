#pragma once

/**
 * @file WindowRegistry.h
 * @brief 进程内窗口注册表：不透明 **NNWindowHandle** ↔ **VGWindow** / `SDL_Window*`。
 *
 * **禁止** 将 `SDL_Window*` 转为句柄；句柄由原子计数器单调分配。
 */

#include "Core/Window.h"

#include "NNNativeEngineAPI/Include/WindowAPI.h"
#include "NNNativeEngineAPI/Include/WindowTypes.h"

struct SDL_Window;

namespace NN::Runtime
{

/**
 * @brief 窗口注册表（单进程、线程安全的基础操作用 mutex 保护）。
 */
class WindowRegistry
{
public:
	/** @brief 按描述创建窗口；失败返回 `NN_INVALID_WINDOW_HANDLE`。 */
	static NNWindowHandle Create(const NNWindowDesc* desc);

	/** @brief 销毁指定句柄；无效句柄为 no-op。 */
	static void Destroy(NNWindowHandle handle);

	/** @brief 销毁全部窗口（Application shutdown 时调用）。 */
	static void DestroyAll();

	/** @brief 解析句柄为 VGWindow；不存在返回 nullptr。 */
	static VGWindow* Resolve(NNWindowHandle handle);

	/** @brief 解析句柄为 SDL_Window*；不存在返回 nullptr。 */
	static SDL_Window* TryGetSdlWindow(NNWindowHandle handle);

	/** @brief 当前已注册窗口数量。 */
	static std::size_t Count();

	/** @brief 若尚无主窗口，将 handle 记为主窗口（供 Application 帧泵使用）。 */
	static void AdoptPrimaryIfUnset(NNWindowHandle handle);

	/** @brief 获取主窗口句柄；未设置时返回 `NN_INVALID_WINDOW_HANDLE`。 */
	static NNWindowHandle GetPrimaryHandle();
};

} // namespace NN::Runtime
