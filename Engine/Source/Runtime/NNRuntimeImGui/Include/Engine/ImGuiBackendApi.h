/*
 * This source file is part of Neverness Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 * See the LICENSE file in the project root for details.
 *
 * ImGui Backend 极小封装 — 只暴露 SDL3/Diligent 后端的初始化、帧边界、事件处理。
 * 不管理 ImGui 生命周期（由 C# Hexa.NET.ImGui 管理）。
 *
 * 从 NNRuntimeApplication 迁移到 NNRuntimeImGui (2026-06-24)。
 */

#pragma once
#include "../imconfig.h"
#include "NNPlatformCore/Include/SDL3/SDL3Window.h"
#include <cstdint>

// ── NNImGuiBackendAPI 类型定义（从 NNAPI/Include/Engine/ImGuiBackendAPI.h 迁入） ──

#if defined(_WIN32)
#define NN_IMGUI_BACKEND_STDCALL __stdcall
#else
#define NN_IMGUI_BACKEND_STDCALL
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** ImGui 后端 API 函数表。 */
typedef struct NNImGuiBackendAPI
{
    std::uint32_t size;
    bool (NN_IMGUI_BACKEND_STDCALL *initialize)(void* sdlWindow, void* device, void* context, void* swapChain);
    void (NN_IMGUI_BACKEND_STDCALL *shutdown)(void);
    void (NN_IMGUI_BACKEND_STDCALL *newFrame)(int width, int height, int preTransform);
    void (NN_IMGUI_BACKEND_STDCALL *render)(void* context, void* swapChain);
    bool (NN_IMGUI_BACKEND_STDCALL *processEvent)(const void* event);
} NNImGuiBackendAPI;

/** @brief 填充 NNImGuiBackendAPI 函数表（由 BuildImGuiBackendApi.cpp 实现）。 */
	IMGUI_API void NNBuildImGuiBackendRuntimeApi(NNImGuiBackendAPI* api);

/**
 * @brief 初始化 ImGui SDL3 + Diligent 后端。
 * @param sdlWindow   SDL_Window 指针
 * @param device      Diligent IRenderDevice* 原生指针
 * @param context     Diligent IDeviceContext* 原生指针
 * @param swapChain   Diligent ISwapChain* 原生指针
 * @return 成功返回 true
 */
	IMGUI_API bool nn_imgui_backend_initialize(
    SDL_Window* sdlWindow,
    void* device,
    void* context,
    void* swapChain);

/**
 * @brief 关闭 ImGui 后端（释放 backend 资源，不销毁 ImGui context）。
 */
	IMGUI_API void nn_imgui_backend_shutdown(void);

/**
 * @brief ImGui NewFrame（SDL3 后端 + Diligent 后端）。
 * @param width       SwapChain 宽度
 * @param height      SwapChain 高度
 * @param preTransform 旋转预变换（Diligent SurfaceTransform）
 */
	IMGUI_API void nn_imgui_backend_new_frame(
    int width,
    int height,
    int preTransform);

/**
 * @brief ImGui Render（设置渲染目标 + 提交绘制数据到 Diligent）。
 * @param context     Diligent IDeviceContext* 原生指针
 * @param swapChain   Diligent ISwapChain* 原生指针
 */
	IMGUI_API void nn_imgui_backend_render(
    void* context,
    void* swapChain);

/**
 * @brief 将 SDL_Event 传递给 ImGui SDL3 后端处理输入。
 * @param event       SDL_Event 指针
 * @return 如果事件被 ImGui 消费返回 true
 */
	IMGUI_API bool nn_imgui_backend_process_event(const SDL_Event* event);

#ifdef __cplusplus
}
#endif
