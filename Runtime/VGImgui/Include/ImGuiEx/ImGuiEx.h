#pragma once
#include <string>
#include <VGImgui/IncludeImGui.h>
#include "../imgui/imgui_internal.h"
#include "IconFont/IconsFontAwesome5Pro.h"
#include "ImGuiExDrawList.h"
#include <functional>
#include <HCore/Include/Core/HCoreTypes.h>

#include "ImScoped.h"
#include "ImWindow.h"
#include "ImNotify.h"
#include "ImTaskManager.h"

namespace ImGuiEx
{
#define IMGUIEX_STYLE_VAR(style, val) ImGuiEx::ScopedStyleVar __##style(style,val);
	//#define IMGUIEX_STYLE_COLOR(col, val) ImGuiEx::ScopedStyleColor __ImGuiStyleVar_##col(ImGuiStyleVar_##col,val);

	IMGUI_API void Initialize();
	IMGUI_API void Render();

		// From imgui/misc/cpp/imgui_stdlib.h
		// ImGui::InputText() with std::string
		// Because text input needs dynamic resizing, we need to setup a callback to grow the capacity
	IMGUI_API bool  InputText(const char* label, std::string& str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
	IMGUI_API bool  InputTextMultiline(const char* label, std::string& str, const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
	IMGUI_API bool  InputTextWithHint(const char* label, const char* hint, std::string& str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);

	//IMGUI_API bool InputSearch(const char* label, const char* hint, std::string& str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = 0, void* user_data = 0);

	IMGUI_API void Image(int renderID, float width = 200.0f, float height = 200.0f);
	IMGUI_API void Image(ImTextureID renderID, float width = 200.0f, float height = 200.0f);
	IMGUI_API void Image(void* renderID, float width = 200.0f, float height = 200.0f);
	IMGUI_API void ImageGL(int renderID, float width = 200.0f, float height = 200.0f);
	IMGUI_API void ImageGL(void* renderID, float width = 200.0f, float height = 200.0f);

	IMGUI_API std::string ConvertRGBHex(const float col[4]);
	IMGUI_API std::string ConvertRGBAHex(const float col[4]);

	IMGUI_API bool StringConvertRGBHex(const std::string& str, ImColor& color);
	IMGUI_API bool StringConvertRGBAHex(const std::string& str, ImColor& color);

	IMGUI_API void SplitLayout(const char* str_id, ImGuiTableFlags flags, std::initializer_list<std::function<void()>> list);

	// Use 'cond' to choose to submit payload on drag start or every frame
	IMGUI_API bool SendDragDropPayload(const char* type, const void* data, size_t data_size, ImGuiCond cond = 0);

	IMGUI_API void PushAllColorsCustom(ImGuiStyle custom_gui_style);
	IMGUI_API void PopAllColorsCustom();

	IMGUI_API void DrawObjBorderFloat2(Horizon::float2 obj_pos, Horizon::float2 obj_size, float distance_thickness = 5.f, ImU32 col = IM_COL32(255, 255, 0, 255), float rounding = 0.f, float tickness = 1.0f);
	IMGUI_API void DrawObjBorder(ImVec2 obj_pos, ImVec2 obj_size, float distance_thickness = 5.f, ImU32 col = IM_COL32(255, 255, 0, 255), float rounding = 0.f, float tickness = 1.0f);

}
