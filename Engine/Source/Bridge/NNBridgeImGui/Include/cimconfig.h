#pragma once

#undef NDEBUG

#if defined(_WIN32) || defined(_WIN64)

#if defined(NN_BRIDGE_IMGUI_EXPORT)
#define NN_BRIDGE_IMGUI_API __declspec(dllexport)
#elif defined(NN_BRIDGE_IMGUI_STATIC)
#define NN_BRIDGE_IMGUI_API
#else
#define NN_BRIDGE_IMGUI_API __declspec(dllimport)
#endif

#else

#if defined(VG_PACKAGE_API_EXPORT)
#define NN_BRIDGE_IMGUI_API __attribute__((visibility("default")))
#else
#define NN_BRIDGE_IMGUI_API
#endif

#endif

#define CIMGUI_VARGS0
