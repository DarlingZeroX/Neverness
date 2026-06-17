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

} NNDiligentAPI;

#ifdef __cplusplus
} /* extern "C" */
#endif
