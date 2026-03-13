#include "pch.h"
#include "ImGuiEx/ImScoped.h"

namespace ImGuiEx
{
	ScopedID::ScopedID(const char* str_id)
	{
		ImGui::PushID(str_id);
	}

	ScopedID::ScopedID(const char* str_id_begin, const char* str_id_end)
	{
		ImGui::PushID(str_id_begin, str_id_end);
	}

	ScopedID::ScopedID(const void* ptr_id)
	{
		ImGui::PushID(ptr_id);
	}

	ScopedID::ScopedID(int int_id)
	{
		ImGui::PushID(int_id);
	}

	ScopedID::~ScopedID()
	{
		ImGui::PopID();
	}

	ScopedStyleVar::ScopedStyleVar(ImGuiStyleVar style, float val)
	{
		ImGui::PushStyleVar(style, val);
	}

	ScopedStyleVar::ScopedStyleVar(ImGuiStyleVar style, const ImVec2& val)
	{
		ImGui::PushStyleVar(style, val);
	}

	ScopedStyleVar::~ScopedStyleVar()
	{
		ImGui::PopStyleVar();
	}

	ScopedStyleColor::ScopedStyleColor(ImGuiCol col, const ImVec4& val)
	{
		ImGui::PushStyleColor(col, val);
	}

	ScopedStyleColor::ScopedStyleColor(ImGuiCol col, ImU32 val)
	{
		ImGui::PushStyleColor(col, val);
	}

	ScopedStyleColor::~ScopedStyleColor()
	{
		ImGui::PopStyleColor();
	}

	ScopedStyle::ScopedStyle() noexcept
		:styleVarCount(0),
		styleColorCount(0)
	{
	}

	void ScopedStyle::PushStyleVar(ImGuiStyleVar style, float val)
	{
		ImGui::PushStyleVar(style, val);
		++styleVarCount;
	}

	void ScopedStyle::PushStyleVar(ImGuiStyleVar style, const ImVec2& val)
	{
		ImGui::PushStyleVar(style, val);
		++styleVarCount;
	}

	void ScopedStyle::PushStyleColor(ImGuiCol col, const ImVec4& val)
	{
		ImGui::PushStyleColor(col, val);
		++styleColorCount;
	}

	void ScopedStyle::PushStyleColor(ImGuiCol col, ImU32 val)
	{
		ImGui::PushStyleColor(col, val);
		++styleColorCount;
	}

	ScopedStyle::~ScopedStyle()
	{
		ImGui::PopStyleVar(styleVarCount);
		ImGui::PopStyleColor(styleColorCount);
	}

	ScopedGroup::ScopedGroup()
	{
		ImGui::BeginGroup();
	}

	ScopedGroup::~ScopedGroup()
	{
		ImGui::EndGroup();
	}

	ScopedItemWidth::ScopedItemWidth(float item_width)
	{
		ImGui::PushItemWidth(item_width);
	}

	ScopedItemWidth::~ScopedItemWidth()
	{
		ImGui::PopItemWidth();
	}

	bool ScopedBeginBase::IsBegin() const noexcept
	{
		return m_bBegin;
	}

	bool& ScopedBeginBase::begin() noexcept
	{
		return m_bBegin;
	}

	ScopedWindow::ScopedWindow(const char* name, bool* p_open, ImGuiWindowFlags flags)
	{
		begin() = ImGui::Begin(name, p_open, flags);
	}

	ScopedWindow::~ScopedWindow()
	{
		ImGui::End();

		begin() = false;
	}

	ScopedPopup::ScopedPopup(const char* str_id, ImGuiWindowFlags flags)
	{
		begin() = ImGui::BeginPopup(str_id, flags);
	}

	ScopedPopup::~ScopedPopup()
	{
		if (IsBegin())
		{
			ImGui::EndPopup();
		}

		begin() = false;
	}

	ScopedDragDropTarget::ScopedDragDropTarget()
	{
		begin() = ImGui::BeginDragDropTarget();
	}

	ScopedDragDropTarget::~ScopedDragDropTarget()
	{
		if (IsBegin())
		{
			ImGui::EndDragDropTarget();
		}

		begin() = false;
	}

	ScopedDragDropSource::ScopedDragDropSource(ImGuiDragDropFlags flags)
	{
		begin() = ImGui::BeginDragDropSource(flags);
	}

	ScopedDragDropSource::~ScopedDragDropSource()
	{
		if (IsBegin())
		{
			ImGui::EndDragDropSource();
		}

		begin() = false;
	}
}