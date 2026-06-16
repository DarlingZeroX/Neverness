#pragma once

/**
 * @file ViewportSurfaceAPI.h
 * @brief 视口 Surface API：管理 SwapChain 生命周期（Renderer 基础设施）。
 *
 * 职责：Create / Destroy / Resize / FlushResizes / Present / SurfaceLost / Recreate
 * 与 ViewportRenderAPI 分离：Surface 是 Renderer 基础设施，RenderViewport 是 Editor 功能。
 * 所有 Viewport（Scene / Material / Mesh / Texture / Shader）共用此 API。
 *
 * v23 新增子表。
 */

#include "NativeInterop.h"
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

/** 原生窗口句柄类型——跨平台标识 HWND / X11 / NSView。 */
typedef enum NNNativeHandleType
{
    /** Win32 HWND（Windows）。 */
    NN_NATIVE_HANDLE_TYPE_WIN32_HWND  = 0,
    /** X11 Window ID（Linux X11）。 */
    NN_NATIVE_HANDLE_TYPE_X11_WINDOW  = 1,
    /** Wayland wl_surface（Linux Wayland）。 */
    NN_NATIVE_HANDLE_TYPE_WAYLAND_SURFACE = 2,
    /** NSView 指针（macOS）。 */
    NN_NATIVE_HANDLE_TYPE_NS_VIEW     = 3
} NNNativeHandleType;

typedef struct NNViewportSurfaceAPI
{
    /**
     * @brief 创建视口表面（SwapChain）。
     * @param nativeHandle  原生窗口句柄（HWND / X11 Window / NSView）
     * @param handleType    句柄类型（NNNativeHandleType）
     * @param width         初始宽度
     * @param height        初始高度
     * @return surfaceId（0 = 失败）
     */
    std::uint64_t (NN_ENGINE_ABI_STDCALL *CreateSurface)(
        void* nativeHandle,
        NNNativeHandleType handleType,
        std::uint32_t width,
        std::uint32_t height);

    /**
     * @brief 销毁视口表面。
     * @param surfaceId  表面 ID
     */
    void (NN_ENGINE_ABI_STDCALL *DestroySurface)(
        std::uint64_t surfaceId);

    /**
     * @brief 标记 Deferred Resize（不立即执行 ResizeBuffers）。
     * @param surfaceId  表面 ID
     * @param width      新宽度
     * @param height     新高度
     */
    void (NN_ENGINE_ABI_STDCALL *MarkResize)(
        std::uint64_t surfaceId,
        std::uint32_t width,
        std::uint32_t height);

    /**
     * @brief 帧末统一执行所有 Deferred Resize（Flush Resize Queue）。
     */
    void (NN_ENGINE_ABI_STDCALL *FlushResizes)(void);

    /**
     * @brief Present SwapChain（提交渲染结果到屏幕）。
     * @param surfaceId  表面 ID
     */
    void (NN_ENGINE_ABI_STDCALL *Present)(
        std::uint64_t surfaceId);

    /**
     * @brief 查询表面是否丢失（HWND 重建后需要 Recreate SwapChain）。
     * @param surfaceId  表面 ID
     * @return 1 = 丢失，0 = 正常
     */
    std::uint8_t (NN_ENGINE_ABI_STDCALL *IsSurfaceLost)(
        std::uint64_t surfaceId);

    /**
     * @brief 重建丢失的表面（新 HWND）。
     * @param surfaceId     表面 ID
     * @param newHandle     新原生窗口句柄
     * @param newHandleType 新句柄类型
     * @return 1 = 成功，0 = 失败
     */
    std::uint8_t (NN_ENGINE_ABI_STDCALL *RecreateSurface)(
        std::uint64_t surfaceId,
        void* newHandle,
        NNNativeHandleType newHandleType);

    /**
     * @brief 渲染视口到 SwapChain（完整路径：SceneRenderer → FBO → CopyTexture → SwapChain → Present）。
     * @param surfaceId   表面 ID
     * @param sceneHandle 场景句柄
     * @param width       渲染宽度
     * @param height      渲染高度
     * @return 1 = 成功，0 = 失败
     */
    std::uint8_t (NN_ENGINE_ABI_STDCALL *RenderViewport)(
        std::uint64_t surfaceId,
        std::uint64_t sceneHandle,
        std::uint32_t width,
        std::uint32_t height);

} NNViewportSurfaceAPI;

#ifdef __cplusplus
} /* extern "C" */
#endif
