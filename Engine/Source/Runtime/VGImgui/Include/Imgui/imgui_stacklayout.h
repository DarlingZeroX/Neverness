// dear imgui, v1.86 WIP
// (stack layout headers)

/*

Index of this file:
// [SECTION] Stack Layout API

*/

#pragma once

#include "imgui.h"
#include "imgui_internal.h"


struct ImGuiWindow;
typedef int ImGuiLayoutType;

namespace ImGui {

    IMGUI_API ImGuiLayoutType GetCurrentStackLayoutType(ImGuiID window_id);
    IMGUI_API void UpdateStackLayoutItemRect(ImGuiID window_id, const ImVec2& min, const ImVec2& max);
    IMGUI_API bool ImGui_ItemSize_StackLayoutHorizontal(ImGuiWindow* window, const ImVec2& size, float text_baseline_y);

    IMGUI_API void BeginHorizontal(const char* str_id, const ImVec2& size = ImVec2(0, 0), float align = -1.0f);
    IMGUI_API void BeginHorizontal(const void* ptr_id, const ImVec2& size = ImVec2(0, 0), float align = -1.0f);
    IMGUI_API void BeginHorizontal(int id, const ImVec2& size = ImVec2(0, 0), float align = -1);
    IMGUI_API void EndHorizontal();
    IMGUI_API void BeginVertical(const char* str_id, const ImVec2& size = ImVec2(0, 0), float align = -1.0f);
    IMGUI_API void BeginVertical(const void* ptr_id, const ImVec2& size = ImVec2(0, 0), float align = -1.0f);
    IMGUI_API void BeginVertical(int id, const ImVec2& size = ImVec2(0, 0), float align = -1);
    IMGUI_API void EndVertical();
    IMGUI_API void Spring(float weight = 1.0f, float spacing = -1.0f);
    IMGUI_API void SuspendLayout();
    IMGUI_API void ResumeLayout();

} // namespace ImGui


