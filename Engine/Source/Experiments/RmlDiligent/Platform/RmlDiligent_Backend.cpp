// RmlDiligent Backend — 实现 RmlUi 官方 Backend:: API，渲染由 RmlDiligentRenderInterface 承担。
#include "RmlDiligent_AppCommon.h"
#include "RmlDiligentRenderInterface.h"

#include "RmlUi_Backend.h"
#include "RmlUi_Platform_SDL.h"

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Log.h>

#include <SDL3/SDL.h>
#include <string>

namespace {

struct BackendData {
    SystemInterface_SDL system_interface;
    RmlDiligent::RmlDiligentRenderInterface render_interface;

    SDL_Window* window = nullptr;
    Diligent::RefCntAutoPtr<Diligent::IRenderDevice> device;
    Diligent::RefCntAutoPtr<Diligent::IDeviceContext> context;
    Diligent::RefCntAutoPtr<Diligent::ISwapChain> swap_chain;

    bool running = true;
};

static BackendData* g_Data = nullptr;

} // namespace

bool Backend::Initialize(const char* window_name, int width, int height, bool allow_resize)
{
    if (g_Data) {
        return false;
    }

    SDL_Window* window = nullptr;
    if (!RmlDiligent::CreateSDLWindow(window_name, width, height, allow_resize, &window)) {
        return false;
    }

    Diligent::IRenderDevice* device = nullptr;
    Diligent::IDeviceContext* context = nullptr;
    Diligent::ISwapChain* swap_chain = nullptr;
    if (!RmlDiligent::CreateDiligentDeviceAndSwapChain(window, &device, &context, &swap_chain)) {
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }

    g_Data = new BackendData();
    g_Data->window = window;
    g_Data->system_interface.SetWindow(window);
    g_Data->device = device;
    g_Data->context = context;
    g_Data->swap_chain = swap_chain;

    if (!g_Data->render_interface.Initialize(device, context, swap_chain)) {
        SDL_DestroyWindow(window);
        delete g_Data;
        g_Data = nullptr;
        device->Release();
        context->Release();
        swap_chain->Release();
        SDL_Quit();
        return false;
    }

    int pixel_w = 0;
    int pixel_h = 0;
    SDL_GetWindowSizeInPixels(window, &pixel_w, &pixel_h);
    g_Data->render_interface.SetProjectionMatrix(pixel_w, pixel_h);

    std::cout << "[RmlDiligent] Backend initialized (Diligent render interface active)" << std::endl;

    return true;
}

void Backend::Shutdown()
{
    if (!g_Data) {
        return;
    }

    SDL_DestroyWindow(g_Data->window);
    delete g_Data;
    g_Data = nullptr;

    SDL_Quit();
}

Rml::SystemInterface* Backend::GetSystemInterface()
{
    return g_Data ? &g_Data->system_interface : nullptr;
}

Rml::RenderInterface* Backend::GetRenderInterface()
{
    return g_Data ? &g_Data->render_interface : nullptr;
}

bool Backend::ProcessEvents(Rml::Context* context, KeyDownCallback key_down_callback, bool power_save)
{
    if (!g_Data || !context) {
        return false;
    }

    auto GetKey = [](const SDL_Event& event) { return event.key.key; };
    auto GetDisplayScale = []() { return SDL_GetWindowDisplayScale(g_Data->window); };

    bool result = g_Data->running;
    g_Data->running = true;

    SDL_Event ev;
    bool has_event = false;
    if (power_save) {
        has_event = SDL_WaitEventTimeout(&ev, static_cast<int>(Rml::Math::Min(context->GetNextUpdateDelay(), 10.0) * 1000));
    } else {
        has_event = SDL_PollEvent(&ev);
    }

    while (has_event) {
        bool propagate_event = true;
        switch (ev.type) {
        case SDL_EVENT_QUIT:
            propagate_event = false;
            result = false;
            break;
        case SDL_EVENT_KEY_DOWN: {
            propagate_event = false;
            const Rml::Input::KeyIdentifier key = RmlSDL::ConvertKey(GetKey(ev));
            const int key_modifier = RmlSDL::GetKeyModifierState();
            const float native_dp_ratio = GetDisplayScale();

            if (key_down_callback && !key_down_callback(context, key, key_modifier, native_dp_ratio, true)) {
                break;
            }
            if (!RmlSDL::InputEventHandler(context, g_Data->window, ev)) {
                break;
            }
            if (key_down_callback && !key_down_callback(context, key, key_modifier, native_dp_ratio, false)) {
                break;
            }
        } break;
        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: {
            const int w = ev.window.data1;
            const int h = ev.window.data2;
            if (w > 0 && h > 0) {
                g_Data->swap_chain->Resize(static_cast<Diligent::Uint32>(w), static_cast<Diligent::Uint32>(h));
                context->SetDimensions(Rml::Vector2i(w, h));
                g_Data->render_interface.SetProjectionMatrix(w, h);
            }
        } break;
        default:
            break;
        }

        if (propagate_event) {
            RmlSDL::InputEventHandler(context, g_Data->window, ev);
        }

        has_event = SDL_PollEvent(&ev);
    }

    return result;
}

void Backend::RequestExit()
{
    if (g_Data) {
        g_Data->running = false;
    }
}

void Backend::BeginFrame()
{
    if (g_Data) {
        g_Data->render_interface.BeginFrame();
    }
}

void Backend::PresentFrame()
{
    if (!g_Data) {
        return;
    }
    g_Data->render_interface.EndFrame();
    g_Data->swap_chain->Present(0);
}
