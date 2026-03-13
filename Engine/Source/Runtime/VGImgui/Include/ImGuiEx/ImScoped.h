#pragma once
#include <functional>
#include <string>
#include "../imgui/imgui.h"

namespace ImGuiEx
{
	struct IMGUI_API ScopedBase
	{
		virtual ~ScopedBase() = default;
	};

	class IMGUI_API ScopedID : public ScopedBase
	{
	public:
		ScopedID(const char* str_id);
		ScopedID(const char* str_id_begin, const char* str_id_end);
		ScopedID(const void* ptr_id);
		ScopedID(int int_id);

		~ScopedID() override;
	};

	class IMGUI_API ScopedStyleVar : public ScopedBase
	{
	public:
		ScopedStyleVar(ImGuiStyleVar style, float val);

		ScopedStyleVar(ImGuiStyleVar style, const ImVec2& val);

		~ScopedStyleVar() override;
	};

	class IMGUI_API ScopedStyleColor : public ScopedBase
	{
	public:
		ScopedStyleColor(ImGuiCol col, const ImVec4& val);

		ScopedStyleColor(ImGuiCol col, ImU32 val);

		~ScopedStyleColor() override;
	};

	class IMGUI_API ScopedStyle : public ScopedBase
	{
	public:
		ScopedStyle() noexcept;

		void PushStyleVar(ImGuiStyleVar style, float val);

		void PushStyleVar(ImGuiStyleVar style, const ImVec2& val);

		void PushStyleColor(ImGuiCol col, const ImVec4& val);

		void PushStyleColor(ImGuiCol col, ImU32 val);

		~ScopedStyle() override;
	private:
		int styleVarCount;
		int styleColorCount;
	};

	class IMGUI_API ScopedGroup final : public ScopedBase
	{
	public:
		ScopedGroup();

		~ScopedGroup() override;
	};

	class IMGUI_API ScopedItemWidth : public ScopedBase
	{
	public:
		ScopedItemWidth(float item_width);

		~ScopedItemWidth() override;
	};

	struct IMGUI_API ScopedBeginBase : public ScopedBase
	{
		~ScopedBeginBase() override = default;

		bool IsBegin() const noexcept;
	protected:
		bool& begin() noexcept;
	private:
		bool m_bBegin = false;
	};

	class IMGUI_API ScopedWindow final : public ScopedBeginBase
	{
	public:
		ScopedWindow(const char* name, bool* p_open = 0, ImGuiWindowFlags flags = 0);

		template<typename T>
		ScopedWindow(T func, const char* name, bool* p_open = 0, ImGuiWindowFlags flags = 0)
		{
			if ((begin() = ImGui::Begin(name, p_open, flags)) == true)
			{
				ScopedStyle style;
				func(style);
			}

			this->~ScopedWindow();
		}

		~ScopedWindow() override;
	private:
	};

	class IMGUI_API ScopedPopup : public ScopedBeginBase
	{
	public:
		ScopedPopup(const char* str_id, ImGuiWindowFlags flags = 0);

		~ScopedPopup() override;
	};

	class IMGUI_API ScopedDragDropTarget final : public ScopedBeginBase
	{
	public:
		ScopedDragDropTarget();

		~ScopedDragDropTarget() override;
	};

	class IMGUI_API ScopedDragDropSource final : public ScopedBeginBase
	{
	public:
		ScopedDragDropSource(ImGuiDragDropFlags flags = 0);

		~ScopedDragDropSource() override;
	};


	template<typename T>
	T PopupContextItem(std::function<T()> func, const char* str_id = 0, ImGuiPopupFlags popup_flags = 1)
	{
		T result = static_cast<T>(0);

		if(ImGui::BeginPopupContextItem(str_id, popup_flags))
		{
			result = func();

			ImGui::EndPopup();
		}

		return result;
	}
}