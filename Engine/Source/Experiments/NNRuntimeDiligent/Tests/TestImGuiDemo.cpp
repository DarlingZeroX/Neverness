// TestImGuiDemo.cpp — Phase 4.3: ImGui via Diligent backend
// Creates device, ImGui renderer, renders demo window for 10 frames.

#include <iostream>
#include <SDL3/SDL.h>

#include <NNRuntimeRenderBootstrap/Include/NNRenderBootstrap.h>
#include <NNRuntimeDiligent/Device/NNDiligentDevice.h>
#include <NNRuntimeDiligent/Command/NNDiligentCommandList.h>
#include <NNRuntimeDiligent/ImGui/NNDiligentImGuiRenderer.h>

// Need full definition for unique_ptr destructor
#include <ImGuiImplSDL3.hpp>

#include <imgui.h>

namespace nnr = NN::Runtime::Render;

int main()
{
    std::cout << "=== Phase 4.3: ImGui via Diligent Backend Test ===" << std::endl;

    // Init SDL
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("ImGui Demo", 1280, 720, SDL_WINDOW_RESIZABLE);
    if (!window)
    {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // Create Diligent device
    nnr::NNRenderDeviceCreateInfo devCI{};
    devCI.Window = window;
    devCI.Width  = 1280;
    devCI.Height = 720;
    devCI.EnableValidation = true;
    devCI.Backend = nnr::NNRenderBackendType::Backend_Vulkan;

    auto renderDev = nnr::NNRenderBootstrap::CreateDevice(devCI);
    if (!renderDev)
    {
        std::cerr << "CreateDevice failed!" << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    std::cout << "Device: " << renderDev->GetDeviceInfo().DeviceName << std::endl;

    auto* dilDevice = static_cast<NNDiligent::NNDiligentDevice*>(renderDev.Get());

    // Create ImGui renderer
    NNDiligent::NNDiligentImGuiRenderer imGuiRenderer;
    NNDiligent::NNDiligentImGuiCreateInfo imGuiCI{};
    imGuiCI.pDevice    = dilDevice->GetDiligentDevice();
    imGuiCI.pContext   = dilDevice->GetDiligentContext();
    imGuiCI.pSwapChain = dilDevice->GetDiligentSwapChain();
    imGuiCI.pWindow    = window;
    imGuiCI.EnableDocking = true;

    if (!imGuiRenderer.Initialize(imGuiCI))
    {
        std::cerr << "ImGui renderer init failed!" << std::endl;
        renderDev = nullptr;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    std::cout << "ImGui renderer initialized" << std::endl;

    // Render 10 frames with ImGui demo window
    bool running = true;
    for (int frame = 0; frame < 10 && running; ++frame)
    {
        // Process SDL events
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                running = false;
                break;
            }
            imGuiRenderer.HandleSDLEvent(&event);
        }
        if (!running) break;

        // ImGui frame
        imGuiRenderer.NewFrame();
        ImGui::ShowDemoWindow();

        // Diligent render
        auto* ctx = dilDevice->GetDiligentContext();
        auto* sc  = dilDevice->GetDiligentSwapChain();

        // Clear back buffer
        auto* pRTV = sc->GetCurrentBackBufferRTV();
        const float clearColor[] = { 0.1f, 0.1f, 0.2f, 1.0f };
        ctx->SetRenderTargets(1, &pRTV, nullptr, ::Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        ctx->ClearRenderTarget(pRTV, clearColor, ::Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        // Render ImGui
        imGuiRenderer.Render(ctx);

        // Present
        sc->Present();
    }

    std::cout << "Rendered 10 frames with ImGui demo window" << std::endl;

    // Cleanup
    imGuiRenderer.Shutdown();
    renderDev = nullptr;
    SDL_DestroyWindow(window);
    SDL_Quit();

    std::cout << "=== PASS ===" << std::endl;
    return 0;
}
