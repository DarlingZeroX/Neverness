#pragma once

#include "Common/interface/RefCntAutoPtr.hpp"
#include "Graphics/GraphicsEngine/interface/DeviceContext.h"
#include "Graphics/GraphicsEngine/interface/RenderDevice.h"
#include "Graphics/GraphicsEngine/interface/SwapChain.h"

struct SDL_Window;

namespace RmlDiligent {

bool CreateSDLWindow(const char* title, int width, int height, bool allow_resize, SDL_Window** out_window);
bool CreateDiligentDeviceAndSwapChain(
    SDL_Window* window,
    Diligent::IRenderDevice** device,
    Diligent::IDeviceContext** context,
    Diligent::ISwapChain** swap_chain);

} // namespace RmlDiligent
