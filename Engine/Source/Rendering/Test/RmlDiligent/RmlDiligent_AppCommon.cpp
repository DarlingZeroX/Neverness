#include "RmlDiligent_AppCommon.h"

#include <SDL3/SDL.h>
#include <iostream>

#ifdef D3D12_SUPPORTED
#include "Graphics/GraphicsEngineD3D12/interface/EngineFactoryD3D12.h"
#endif

#ifdef D3D11_SUPPORTED
#include "Graphics/GraphicsEngineD3D11/interface/EngineFactoryD3D11.h"
#endif

namespace RmlDiligent {

bool CreateSDLWindow(const char* title, int width, int height, bool allow_resize, SDL_Window** out_window)
{
    if (!out_window) {
        return false;
    }

    SDL_SetHint(SDL_HINT_IME_IMPLEMENTED_UI, "composition");
    SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");
    SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        std::cerr << "[RmlDiligent] SDL_Init failed: " << SDL_GetError() << std::endl;
        return false;
    }

    const float scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, title);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, SDL_WINDOWPOS_CENTERED);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, SDL_WINDOWPOS_CENTERED);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, static_cast<int>(width * scale));
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, static_cast<int>(height * scale));
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN, allow_resize);
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_HIGH_PIXEL_DENSITY_BOOLEAN, true);

    SDL_Window* window = SDL_CreateWindowWithProperties(props);
    SDL_DestroyProperties(props);

    if (!window) {
        std::cerr << "[RmlDiligent] SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        return false;
    }

    *out_window = window;
    return true;
}

bool CreateDiligentDeviceAndSwapChain(
    SDL_Window* window,
    Diligent::IRenderDevice** device,
    Diligent::IDeviceContext** context,
    Diligent::ISwapChain** swap_chain)
{
    if (!window || !device || !context || !swap_chain) {
        return false;
    }

    SDL_PropertiesID props = SDL_GetWindowProperties(window);
    void* hwnd = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
    if (!hwnd) {
        std::cerr << "[RmlDiligent] Failed to get Win32 HWND from SDL window" << std::endl;
        return false;
    }

    int width = 0;
    int height = 0;
    SDL_GetWindowSizeInPixels(window, &width, &height);
    if (width <= 0 || height <= 0) {
        SDL_GetWindowSize(window, &width, &height);
    }

    Diligent::SwapChainDesc scDesc;
    scDesc.Width = static_cast<Diligent::Uint32>(width);
    scDesc.Height = static_cast<Diligent::Uint32>(height);
    scDesc.ColorBufferFormat = Diligent::TEX_FORMAT_RGBA8_UNORM;
    scDesc.DepthBufferFormat = Diligent::TEX_FORMAT_D32_FLOAT;
    scDesc.Usage = Diligent::SWAP_CHAIN_USAGE_RENDER_TARGET;

#ifdef D3D12_SUPPORTED
    if (auto* factory = Diligent::GetEngineFactoryD3D12()) {
        Diligent::EngineD3D12CreateInfo engineCI;
        engineCI.EnableValidation = false;
        factory->CreateDeviceAndContextsD3D12(engineCI, device, context);
        if (*device) {
            Diligent::Win32NativeWindow nativeWnd{hwnd};
            factory->CreateSwapChainD3D12(*device, *context, scDesc, Diligent::FullScreenModeDesc{}, nativeWnd, swap_chain);
            if (*swap_chain) {
                std::cout << "[RmlDiligent] Backend: D3D12" << std::endl;
                return true;
            }
        }
    }
#endif

#ifdef D3D11_SUPPORTED
    if (auto* factory = Diligent::GetEngineFactoryD3D11()) {
        Diligent::EngineD3D11CreateInfo engineCI;
        engineCI.EnableValidation = false;
        factory->CreateDeviceAndContextsD3D11(engineCI, device, context);
        if (*device) {
            Diligent::Win32NativeWindow nativeWnd{hwnd};
            factory->CreateSwapChainD3D11(*device, *context, scDesc, Diligent::FullScreenModeDesc{}, nativeWnd, swap_chain);
            if (*swap_chain) {
                std::cout << "[RmlDiligent] Backend: D3D11" << std::endl;
                return true;
            }
        }
    }
#endif

    return false;
}

} // namespace RmlDiligent
