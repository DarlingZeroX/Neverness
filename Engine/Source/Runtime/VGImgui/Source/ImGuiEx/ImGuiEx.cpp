#include "pch.h"
#include "ImGuiEx/ImGuiEx.h"
#include <NNKernel/Interface/HVector.h>

namespace ImGuiEx
{
	struct InputTextCallback_UserData
	{
		std::string* Str;
		ImGuiInputTextCallback  ChainCallback;
		void* ChainCallbackUserData;
	};

	static int InputTextCallback(ImGuiInputTextCallbackData* data)
	{
		InputTextCallback_UserData* user_data = (InputTextCallback_UserData*)data->UserData;
		if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
		{
			// Resize string callback
			// If for some reason we refuse the new length (BufTextLen) and/or capacity (BufSize) we need to set them back to what we want.
			std::string* str = user_data->Str;
			IM_ASSERT(data->Buf == str->c_str());
			str->resize(data->BufTextLen);
			data->Buf = (char*)str->c_str();
		}
		else if (user_data->ChainCallback)
		{
			// Forward to user callback, if any
			data->UserData = user_data->ChainCallbackUserData;
			return user_data->ChainCallback(data);
		}
		//ImGui::Image()
		return 0;
	}

	void Initialize()
	{
	}

	void Render()
	{
		ImTaskManager::Get().RenderUITask();
		RenderNotifications();
	}

	bool InputText(const char* label, std::string& str, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
	{
		IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
		flags |= ImGuiInputTextFlags_CallbackResize;

		InputTextCallback_UserData cb_user_data;
		cb_user_data.Str = &str;
		cb_user_data.ChainCallback = callback;
		cb_user_data.ChainCallbackUserData = user_data;
		return ImGui::InputText(label, (char*)str.c_str(), str.capacity() + 1, flags, InputTextCallback, &cb_user_data);
	}

	bool InputTextMultiline(const char* label, std::string& str, const ImVec2& size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
	{
		IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
		flags |= ImGuiInputTextFlags_CallbackResize;

		InputTextCallback_UserData cb_user_data;
		cb_user_data.Str = &str;
		cb_user_data.ChainCallback = callback;
		cb_user_data.ChainCallbackUserData = user_data;
		return ImGui::InputTextMultiline(label, (char*)str.c_str(), str.capacity() + 1, size, flags, InputTextCallback, &cb_user_data);
	}

	bool InputTextWithHint(const char* label, const char* hint, std::string& str, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
	{
		IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
		flags |= ImGuiInputTextFlags_CallbackResize;

		InputTextCallback_UserData cb_user_data;
		cb_user_data.Str = &str;
		cb_user_data.ChainCallback = callback;
		cb_user_data.ChainCallbackUserData = user_data;
		return ImGui::InputTextWithHint(label, hint, (char*)str.c_str(), str.capacity() + 1, flags, InputTextCallback, &cb_user_data);
	}

	//bool InputSearch(const char* label, const char* hint, std::string& str, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
	//{
	//	char buf[100];
	//	memset(buf, '\0', 100);
	//	memcpy(buf, str.data(), str.size());
	//
	//	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8);
	//	std::string hintText = std::string(ICON_FA_SEARCH);
	//	bool res = ImGui::InputTextWithHint(label, hint, buf, 100, flags, callback, user_data);
	//	ImGui::PopStyleVar();
	//
	//	str = buf;
	//	return res;
	//}

	void Image(int renderID, float width, float height)
	{
		ImGui::Image(
			renderID,
			ImVec2{ width, height },
			ImVec2{ 0,1 },
			ImVec2{ 1,0 }
		);
	}

	void Image(ImTextureID renderID, float width, float height)
	{
		ImGui::Image(
			renderID,
			ImVec2{ width, height }
		);
	}

	void Image(void* renderID, float width, float height)
	{
		ImGui::Image(
			reinterpret_cast<ImTextureID>( renderID ),
			ImVec2{ width, height },
			ImVec2{ 0,1 },
			ImVec2{ 1,0 }
		);
	}

	void ImageGL(int renderID, float width, float height)
	{
		ImGui::Image(
			renderID,
			ImVec2{ width, height },
			ImVec2{ 0,0 },
			ImVec2{ 1,1 }
		);
	}

	void ImageGL(void* renderID, float width, float height)
	{
		ImGui::Image(
			reinterpret_cast<ImTextureID>(renderID),
			ImVec2{ width, height },
			ImVec2{ 0,0 },
			ImVec2{ 1,1 }
		);
	}

	std::string ConvertRGBHex(const float col[4])
	{
		int i[4] = { IM_F32_TO_INT8_UNBOUND(col[0]), IM_F32_TO_INT8_UNBOUND(col[1]), IM_F32_TO_INT8_UNBOUND(col[2]), IM_F32_TO_INT8_UNBOUND(1.0f) };

		char buf[64];
		ImFormatString(buf, IM_ARRAYSIZE(buf), "#%02X%02X%02X", ImClamp(i[0], 0, 255), ImClamp(i[1], 0, 255), ImClamp(i[2], 0, 255));

		return buf;
	}

	std::string ConvertRGBAHex(const float col[4])
	{
		int i[4] = { IM_F32_TO_INT8_UNBOUND(col[0]), IM_F32_TO_INT8_UNBOUND(col[1]), IM_F32_TO_INT8_UNBOUND(col[2]), IM_F32_TO_INT8_UNBOUND(col[3]) };

		char buf[64];
		ImFormatString(buf, IM_ARRAYSIZE(buf), "#%02X%02X%02X%02X", ImClamp(i[0], 0, 255), ImClamp(i[1], 0, 255), ImClamp(i[2], 0, 255), ImClamp(i[3], 0, 255));

		return buf;
	}

	unsigned short CharConvertNumber(char channel)
	{
		if (channel <= 'z')
		{
			if ('a' <= channel)
				return 0xA + channel - 'a';

			if (channel <= 'Z')
			{
				if ('A' <= channel)
					return 0xA + channel - 'A';

				if ('0' <= channel && channel <= '9')
					return channel - '0';
			}
		}

		return 0;
	}

	float Char2ConvertColor(const char* channel)
	{
		const char c1 = channel[0];
		const char c2 = channel[1];

		const auto n2 = CharConvertNumber(c1);
		const auto n1 = CharConvertNumber(c2);

		const auto value = n2 * 16 + n1;

		return static_cast<float>(value) * (1.f / 255.f);
	}

	bool StringConvertRGBHex(const std::string& str, ImColor& color)
	{
		if (str.size() != 7)
			return false;

		color.Value.x = Char2ConvertColor(&str.c_str()[1]);
		color.Value.y = Char2ConvertColor(&str.c_str()[3]);
		color.Value.z = Char2ConvertColor(&str.c_str()[5]);

		return true;
	}

	bool StringConvertRGBAHex(const std::string& str, ImColor& color)
	{
		if (str.size() != 9)
			return false;

		color.Value.x = Char2ConvertColor(&str.c_str()[1]);
		color.Value.y = Char2ConvertColor(&str.c_str()[3]);
		color.Value.z = Char2ConvertColor(&str.c_str()[5]);
		color.Value.w = Char2ConvertColor(&str.c_str()[7]);

		return true;
	}

	void SplitLayout(const char* str_id, ImGuiTableFlags flags, std::initializer_list<std::function<void()>> list)
	{
		if (ImGui::BeginTable(str_id, static_cast<int>(list.size()), flags) == true)
		{
			for (auto& item : list)
			{
				ImGui::TableNextColumn();
				item();
			}

			ImGui::EndTable();
		}
	}

	bool SendDragDropPayload(const char* type, const void* data, size_t data_size, ImGuiCond cond)
	{
		bool result = false;

		if (ImGui::BeginDragDropSource())
		{
			result = ImGui::SetDragDropPayload(type, data, data_size, cond);

			ImGui::EndDragDropSource();
		}

		return result;
	}

	void PushAllColorsCustom(ImGuiStyle custom_gui_style)
	{
		auto& style = ImGui::GetStyle();
		//style.Colors = custom_gui_style.Colors;

		for (auto i = 0; i < ImGuiCol_COUNT; i++)
		{
			ImGui::PushStyleColor(static_cast<ImGuiCol_>(i), custom_gui_style.Colors[i]);
		}
		//Padding
		style.WindowPadding = custom_gui_style.WindowPadding;
		style.FramePadding = custom_gui_style.FramePadding;
		style.CellPadding = custom_gui_style.CellPadding;
		style.ItemSpacing = custom_gui_style.ItemSpacing;
		style.ItemInnerSpacing = custom_gui_style.ItemInnerSpacing;
		style.TouchExtraPadding = custom_gui_style.TouchExtraPadding;
		style.IndentSpacing = custom_gui_style.IndentSpacing;
		style.ScrollbarSize = custom_gui_style.ScrollbarSize;
		style.GrabMinSize = custom_gui_style.GrabMinSize;
		style.WindowBorderSize = custom_gui_style.WindowBorderSize;
		style.ChildBorderSize = custom_gui_style.ChildBorderSize;
		style.PopupBorderSize = custom_gui_style.PopupBorderSize;
		style.FrameBorderSize = custom_gui_style.FrameBorderSize;
		style.TabBorderSize = custom_gui_style.TabBorderSize;

		//Rounding
		style.WindowRounding = custom_gui_style.WindowRounding;
		style.ChildRounding = custom_gui_style.ChildRounding;
		style.FrameRounding = custom_gui_style.FrameRounding;
		style.PopupRounding = custom_gui_style.PopupRounding;
		style.ScrollbarRounding = custom_gui_style.ScrollbarRounding;
		style.GrabRounding = custom_gui_style.GrabRounding;
		style.LogSliderDeadzone = custom_gui_style.LogSliderDeadzone;
		style.TabRounding = custom_gui_style.TabRounding;

		//Position
		style.WindowTitleAlign = custom_gui_style.WindowTitleAlign;
		style.WindowMenuButtonPosition = custom_gui_style.WindowMenuButtonPosition;
		style.ColorButtonPosition = custom_gui_style.ColorButtonPosition;
		style.ButtonTextAlign = custom_gui_style.ButtonTextAlign;
		style.SelectableTextAlign = custom_gui_style.SelectableTextAlign;

		//NANI???
		style.DisplaySafeAreaPadding = custom_gui_style.DisplaySafeAreaPadding;
	}

	void PopAllColorsCustom()
	{
		ImGui::PopStyleColor(ImGuiCol_COUNT);
	}

	void DrawObjBorderFloat2(Horizon::float2 obj_pos, Horizon::float2 obj_size, float distance_thickness, ImU32 col, float rounding, float tickness)
	{
		DrawObjBorder(ImVec2(obj_pos.x, obj_pos.y), ImVec2(obj_size.x, obj_size.y), distance_thickness, col, rounding, tickness);
	}

	void DrawObjBorder(ImVec2 obj_pos, ImVec2 obj_size, float distance_thickness, ImU32 col, float rounding, float tickness)
	{
		ImVec2 vMin = { ImGui::GetWindowPos().x + obj_pos.x - distance_thickness, ImGui::GetWindowPos().y + obj_pos.y - distance_thickness };
		ImVec2 vMax = { vMin.x + obj_size.x + (distance_thickness * 2.f), vMin.y + obj_size.y + (distance_thickness * 2.f) };
		//ImGui::GetForegroundDrawList( )->AddRect( vMin, vMax, col ); //will draw on top of everything 
		ImGui::GetWindowDrawList()->AddRect(vMin, vMax, col, rounding, 15, tickness);
	}
}
