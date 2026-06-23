/*
 * This source file is part of Neverness Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 * See the LICENSE file in the project root for details.
 *
 * ImGui Backend 极小封装 — 只暴露 SDL3/Diligent 后端函数。
 * 不管理 ImGui 生命周期（由 C# Hexa.NET.ImGui 管理）。
 */

#include "Engine/ImGuiBackendApi.h"

#include <SDL3/SDL.h>

// ImGui 核心
#include <imgui.h>

// Diligent ImGui 后端（内部已包含 IRenderDevice/IDeviceContext/SwapChain 头文件）
#include <ImGuiImplDiligent.hpp>
#include <ImGuiImplSDL3.hpp>

#include "DeviceContext.h"

using namespace Diligent;

// 静态状态（进程内唯一一份）
namespace
{
    std::unique_ptr<ImGuiImplSDL3> g_Backend;
    bool g_Initialized = false;
}

NN_RUNTIME_APPLICATION_API bool nn_imgui_backend_initialize(
    SDL_Window* sdlWindow,
    void* device,
    void* context,
    void* swapChain)
{
    if (g_Initialized)
    {
        return true;
    }

    if (!sdlWindow || !device || !context || !swapChain)
    {
        return false;
    }

    auto* dilDevice = static_cast<IRenderDevice*>(device);
    auto* dilSwapChain = static_cast<ISwapChain*>(swapChain);

    // 获取交换链格式
    const auto& SCDesc = dilSwapChain->GetDesc();

    // 创建 Diligent ImGui 后端（内部会创建 ImGui context 如果不存在）
    ImGuiDiligentCreateInfo CI{dilDevice, SCDesc};
    g_Backend = ImGuiImplSDL3::Create(CI, sdlWindow);

    if (!g_Backend)
    {
        return false;
    }

    g_Initialized = true;
    return true;
}

NN_RUNTIME_APPLICATION_API void nn_imgui_backend_shutdown(void)
{
    if (g_Backend)
    {
        g_Backend.reset();
    }
    g_Initialized = false;
}

NN_RUNTIME_APPLICATION_API void nn_imgui_backend_new_frame(
    int width,
    int height,
    int preTransform)
{
    if (!g_Initialized || !g_Backend)
    {
        return;
    }

    g_Backend->NewFrame(width, height, static_cast<Diligent::SURFACE_TRANSFORM>(preTransform));
}

NN_RUNTIME_APPLICATION_API void nn_imgui_backend_render(
    void* context,
    void* swapChain)
{
    if (!g_Initialized || !g_Backend || !context || !swapChain)
    {
        return;
    }

    auto* dilContext = static_cast<IDeviceContext*>(context);
    auto* dilSwapChain = static_cast<ISwapChain*>(swapChain);

    // Diligent ImGui 后端不会自动绑定渲染目标，
    // 必须在 Render 之前把 SwapChain 的 RTV/DSV 设好
    auto* pRTV = dilSwapChain->GetCurrentBackBufferRTV();
    auto* pDSV = dilSwapChain->GetDepthBufferDSV();
    dilContext->SetRenderTargets(1, &pRTV, pDSV, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    g_Backend->Render(dilContext);

    // 多视口支持
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

NN_RUNTIME_APPLICATION_API bool nn_imgui_backend_process_event(const SDL_Event* event)
{
    if (!g_Initialized || !g_Backend || !event)
    {
        return false;
    }

    return g_Backend->HandleSDLEvent(event);
}
