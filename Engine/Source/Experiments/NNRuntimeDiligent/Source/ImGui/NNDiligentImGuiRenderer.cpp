// NNDiligentImGuiRenderer.cpp — Diligent ImGui backend adapter implementation

#include "../../ImGui/NNDiligentImGuiRenderer.h"

// Diligent ImGui backend
#include <ImGuiImplSDL3.hpp>
#include <ImGuiImplDiligent.hpp>

// ImGui core (from NNRuntimeImGui)
#include <imgui.h>

// Diligent types
#include "Graphics/GraphicsEngine/interface/RenderDevice.h"
#include "Graphics/GraphicsEngine/interface/SwapChain.h"
#include "Graphics/GraphicsEngine/interface/DeviceContext.h"

// SDL
#include <SDL3/SDL.h>

#include <iostream>

namespace NNDiligent
{
    NNDiligentImGuiRenderer::~NNDiligentImGuiRenderer()
    {
        Shutdown();
    }

    bool NNDiligentImGuiRenderer::Initialize(const NNDiligentImGuiCreateInfo& info)
    {
        if (!info.pDevice || !info.pWindow)
        {
            std::cerr << "[NNDiligentImGuiRenderer] Missing device or window" << std::endl;
            return false;
        }

        m_Window = info.pWindow;

        // Setup ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        // Note: Diligent's bundled ImGui does not support docking

        // Setup ImGui style
        ImGui::StyleColorsDark();

        // Create Diligent ImGui backend
        Diligent::ImGuiDiligentCreateInfo ci;
        ci.pDevice = info.pDevice;
        if (info.pSwapChain)
        {
            const auto& scDesc = info.pSwapChain->GetDesc();
            ci.BackBufferFmt  = scDesc.ColorBufferFormat;
            ci.DepthBufferFmt = scDesc.DepthBufferFormat;
        }

        m_Impl = Diligent::ImGuiImplSDL3::Create(ci, info.pWindow);
        if (!m_Impl)
        {
            std::cerr << "[NNDiligentImGuiRenderer] Failed to create ImGuiImplSDL3" << std::endl;
            ImGui::DestroyContext();
            return false;
        }

        m_Initialized = true;
        std::cout << "[NNDiligentImGuiRenderer] Initialized (Diligent backend)" << std::endl;
        return true;
    }

    void NNDiligentImGuiRenderer::Shutdown()
    {
        if (m_Impl)
        {
            m_Impl.reset();
        }
        if (ImGui::GetCurrentContext())
        {
            ImGui::DestroyContext();
        }
        m_Initialized = false;
        m_Window = nullptr;
    }

    void NNDiligentImGuiRenderer::NewFrame()
    {
        if (!m_Impl || !m_Window) return;

        // Get window size for ImGui frame
        int w = 0, h = 0;
        SDL_GetWindowSizeInPixels(m_Window, &w, &h);

        m_Impl->NewFrame(
            static_cast<Diligent::Uint32>(w),
            static_cast<Diligent::Uint32>(h),
            Diligent::SURFACE_TRANSFORM_IDENTITY
        );
    }

    void NNDiligentImGuiRenderer::Render(::Diligent::IDeviceContext* pCtx)
    {
        if (!m_Impl) return;
        m_Impl->Render(pCtx);
    }

    bool NNDiligentImGuiRenderer::HandleSDLEvent(const SDL_Event* event)
    {
        if (!m_Impl) return false;
        return m_Impl->HandleSDLEvent(event);
    }

} // namespace NNDiligent
