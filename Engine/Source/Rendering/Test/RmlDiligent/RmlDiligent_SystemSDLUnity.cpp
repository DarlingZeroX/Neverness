// Unity compile upstream RmlUi_Platform_SDL.cpp with SDL3 only (vcpkg SDL3 may omit SDL_MAJOR_VERSION).
#define RMLUI_SDL_VERSION_MAJOR 3
#include <SDL3/SDL.h>
#ifndef SDL_MAJOR_VERSION
#define SDL_MAJOR_VERSION 3
#endif
#include "../../../ThirdParty/RmlUi/Backends/RmlUi_Platform_SDL.cpp"
