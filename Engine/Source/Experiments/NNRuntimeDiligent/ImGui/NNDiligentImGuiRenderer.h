#pragma once
// NNDiligentImGuiRenderer.h — Diligent ImGui backend adapter for Neverness
// Wraps Diligent's ImGuiImplSDL3 + ImGuiImplDiligent

#include "../NNDiligentConfig.h"
#include <memory>

// Forward declare SDL types
struct SDL_Window;
union SDL_Event;

// Forward declare Diligent types
namespace Diligent
{
    class ImGuiImplSDL3;
    struct IRenderDevice;
    struct IDeviceContext;
    struct ISwapChain;
}

namespace NNDiligent
{
    struct NNDiligentImGuiCreateInfo
    {
        ::Diligent::IRenderDevice*  pDevice    = nullptr;
        ::Diligent::IDeviceContext* pContext   = nullptr;
        ::Diligent::ISwapChain*     pSwapChain = nullptr;
        SDL_Window*                 pWindow    = nullptr;
        bool                        EnableDocking = true;
    };

    class NNDiligentImGuiRenderer
    {
    public:
        NNDiligentImGuiRenderer() = default;
        ~NNDiligentImGuiRenderer();

        // Non-copyable
        NNDiligentImGuiRenderer(const NNDiligentImGuiRenderer&) = delete;
        NNDiligentImGuiRenderer& operator=(const NNDiligentImGuiRenderer&) = delete;

        bool Initialize(const NNDiligentImGuiCreateInfo& info);
        void Shutdown();

        // Frame lifecycle
        void NewFrame();
        void Render(::Diligent::IDeviceContext* pCtx);

        // SDL event forwarding
        bool HandleSDLEvent(const SDL_Event* event);

        bool IsInitialized() const { return m_Initialized; }

    private:
        std::unique_ptr<::Diligent::ImGuiImplSDL3> m_Impl;
        SDL_Window*     m_Window     = nullptr;
        bool            m_Initialized = false;
    };

} // namespace NNDiligent
