#pragma once

/**
 * @file DiligentAPI.h
 * @brief Diligent 底层设备指针暴露 API。
 *
 * 职责：
 * - 获取主窗口的 Diligent IRenderDevice / IDeviceContext / ISwapChain
 * - 创建 ViewportSurface 并直接返回其 ISwapChain*
 *
 * 使用场景：
 * - C# 端通过 NativeAOT 直接调用 Diligent API
 * - 外部渲染模块需要底层设备指针
 *
 * 设计：
 * - 返回 void* 隔离 Diligent 头文件依赖
 * - SwapChain 生命周期由 C# 端管理
 * - v28 新增子表。
 */

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NNDiligentAPI
{
    /**
     * @brief 获取主窗口的 Diligent IRenderDevice*。
     * @return 设备指针（nullptr = 主窗口未初始化或非 Diligent 后端）
     */
    void* (NN_ENGINE_ABI_STDCALL *GetPrimaryDevice)(void);

    /**
     * @brief 获取主窗口的 Diligent IDeviceContext*。
     * @return 上下文指针（nullptr = 主窗口未初始化或非 Diligent 后端）
     */
    void* (NN_ENGINE_ABI_STDCALL *GetPrimaryContext)(void);

    /**
     * @brief 获取主窗口的 Diligent ISwapChain*。
     * @return SwapChain 指针（nullptr = 主窗口未初始化或非 Diligent 后端）
     */
    void* (NN_ENGINE_ABI_STDCALL *GetPrimarySwapChain)(void);

    /**
     * @brief 创建 ViewportSurface 并返回其 ISwapChain*。
     *
     * 与 ViewportSurfaceAPI.CreateSurface 的区别：
     * - CreateSurface 返回 surfaceId（需通过其他 API 获取 SwapChain）
     * - 本 API 直接返回 SwapChain 指针，简化调用链
     *
     * @param nativeHandle  原生窗口句柄（HWND / X11 Window / NSView）
     * @param handleType    句柄类型（NNNativeHandleType）
     * @param width         初始宽度
     * @param height        初始高度
     * @return SwapChain 指针（nullptr = 创建失败）
     *
     * @note SwapChain 生命周期由 C# 端管理。
     */
    void* (NN_ENGINE_ABI_STDCALL *CreateViewportSurfaceWithSwapChain)(
        void* nativeHandle,
        std::uint32_t handleType,
        std::uint32_t width,
        std::uint32_t height);

    /**
     * @brief 为外部创建的 SDL_Window 创建 Diligent 设备。
     *
     * C# 端通过 SDL3-CS 创建窗口后，调用此函数创建 Diligent 设备。
     * 创建的设备会注册为全局主设备，供 GetPrimaryDevice/Context/SwapChain 使用。
     *
     * @param sdlWindow   SDL_Window* 指针（由 C# SDL3-CS 创建）
     * @param width       SwapChain 初始宽度
     * @param height      SwapChain 初始高度
     * @return 1 = 创建成功，0 = 失败
     *
     * @note 设备生命周期由 C++ 端管理，Shutdown 时自动释放。
     * @note 重复调用会被忽略（已有设备时直接返回 1）。
     */
    std::uint32_t (NN_ENGINE_ABI_STDCALL *CreateDeviceForWindow)(
        void* sdlWindow,
        std::uint32_t width,
        std::uint32_t height);

    /**
     * @brief 从平台原生窗口句柄创建 Diligent 设备（绕过 SDL）。
     *
     * C# 端创建 SDL 窗口后，通过 SDL_GetWindowProperties 获取 HWND 等原生句柄，
     * 直接传入此函数创建 Diligent 设备。避免 C#/C++ 各自加载不同 SDL3 实例导致的指针不可共享问题。
     *
     * @param nativeHandle  原生窗口句柄（HWND / X11 Window / NSView 等）
     * @param handleType    句柄类型：0=Win32HWND, 1=X11Window, 2=Wayland, 3=NSView
     * @param width         SwapChain 初始宽度
     * @param height        SwapChain 初始高度
     * @return 1 = 创建成功，0 = 失败
     */
    std::uint32_t (NN_ENGINE_ABI_STDCALL *CreateDeviceForNativeHandle)(
        void* nativeHandle,
        std::uint32_t handleType,
        std::uint32_t width,
        std::uint32_t height);

    /**
     * @brief Present 主 SwapChain。
     *
     * 每帧 EndFrame 时调用，将渲染结果显示到窗口。
     */
    void (NN_ENGINE_ABI_STDCALL *PresentPrimarySwapChain)(void);

    /**
     * @brief 获取主窗口的 INNRenderDevice*（NNRuntimeRender 接口）。
     *
     * 与 GetPrimaryDevice 的区别：
     * - GetPrimaryDevice 返回 Diligent IDevice*（底层设备指针）
     * - GetPrimaryRenderDevice 返回 INNRenderDevice*（NN 封装接口）
     *
     * 使用场景：
     * - Renderer2D 等需要 INNRenderDevice 的模块
     *
     * @return INNRenderDevice 指针（nullptr = 未初始化）
     */
    void* (NN_ENGINE_ABI_STDCALL *GetPrimaryRenderDevice)(void);

} NNDiligentAPI;

#ifdef __cplusplus
} /* extern "C" */
#endif
