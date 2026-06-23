#pragma once

/**
 * @file ImGuiBackendAPI.h
 * @brief ImGui SDL3/Diligent 后端封装。
 *
 * 只暴露后端的初始化、帧边界、事件处理函数。
 * 不管理 ImGui 生命周期（由 C# Hexa.NET.ImGui 管理）。
 * v33 新增。
 */

#include <cstdint>

// 调用约定（与 NativeInterop.h 一致，内联避免跨模块 include 路径问题）
#if defined(_WIN32)
#define NN_IMGUI_BACKEND_STDCALL __stdcall
#else
#define NN_IMGUI_BACKEND_STDCALL
#endif

#ifndef __cplusplus
#include <stdbool.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** ImGui 后端 API 函数表。 */
typedef struct NNImGuiBackendAPI
{
    std::uint32_t size;

    /**
     * @brief 初始化 ImGui SDL3 + Diligent 后端。
     * @param sdlWindow   SDL_Window 指针
     * @param device      Diligent IDevice* 原生指针
     * @param context     Diligent IDeviceContext* 原生指针
     * @param swapChain   Diligent ISwapChain* 原生指针
     * @return 成功返回 true
     */
    bool (NN_IMGUI_BACKEND_STDCALL *initialize)(void* sdlWindow, void* device, void* context, void* swapChain);

    /** @brief 关闭 ImGui 后端。 */
    void (NN_IMGUI_BACKEND_STDCALL *shutdown)(void);

    /**
     * @brief ImGui NewFrame（SDL3 + Diligent 后端）。
     * @param width        SwapChain 宽度
     * @param height       SwapChain 高度
     * @param preTransform 旋转预变换（Diligent SURFACE_TRANSFORM）
     */
    void (NN_IMGUI_BACKEND_STDCALL *newFrame)(int width, int height, int preTransform);

    /**
     * @brief ImGui Render（设置渲染目标 + 提交绘制数据）。
     * @param context      Diligent IDeviceContext* 原生指针
     * @param swapChain    Diligent ISwapChain* 原生指针
     */
    void (NN_IMGUI_BACKEND_STDCALL *render)(void* context, void* swapChain);

    /**
     * @brief 将 SDL_Event 传递给 ImGui 后端处理输入。
     * @param event        SDL_Event 指针
     * @return 如果事件被 ImGui 消费返回 true
     */
    bool (NN_IMGUI_BACKEND_STDCALL *processEvent)(const void* event);

} NNImGuiBackendAPI;

#ifdef __cplusplus
} /* extern "C" */
#endif
