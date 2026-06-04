// imconfig_diligent.h — ImGui config for NNRuntimeDiligent (static build)
// This overrides NNRuntimeImGui's imconfig.h which uses dllimport/dllexport

#pragma once

// Static build — no dllimport/dllexport
#define IMGUI_API

// Enable docking
#define IMGUI_ENABLE_DOCKING

// Enable FreeType font rendering
#define IMGUI_ENABLE_FREETYPE

// Use 32-bit wchar for full Unicode support
#define IMGUI_USE_WCHAR32

// Math operators
#define IMGUI_DEFINE_MATH_OPERATORS
