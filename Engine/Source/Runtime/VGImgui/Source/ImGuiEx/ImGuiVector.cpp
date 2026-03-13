#include "pch.h"
#include "ImGuiEx/ImGuiVector.h"

namespace ImGuiEx
{
	Vec3Dragging DrawVec3Control(
		const std::string& label,
		Horizon::float3& values,
		float resetValue,
		float v_speed,
		float v_min, float v_max
	)
	{
		Vec3Dragging dragging;

		//float lineHeight = ImGui::GetFont()->FontSize + ImGui::GetStyle().FramePadding.y * 2.0f;
		float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
		ImVec2 buttonSize = { 5, lineHeight };

		ImGui::PushID((label + "Vec3Control").c_str());
		//X
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		if (ImGui::Button("##X", buttonSize))
			values.x = resetValue;
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::PushItemWidth(60);
		dragging.x = ImGui::DragFloat("##Drag X", &values.x, v_speed, v_min, v_max, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		//Y
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		if (ImGui::Button("##Y", buttonSize))
			values.y = resetValue;
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::PushItemWidth(60);
		dragging.y = ImGui::DragFloat("##Drag Y", &values.y, v_speed, v_min, v_max, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		//Z
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		if (ImGui::Button("##Z", buttonSize))
			values.z = resetValue;
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::PushItemWidth(60);
		dragging.z = ImGui::DragFloat("##Drag Z", &values.z, v_speed, v_min, v_max, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PopID();

		return dragging;
	}

	IMGUI_API Vec2Dragging DrawVec2Control(const std::string& label, Horizon::float2& values, float resetValue, float v_speed, float v_min, float v_max)
	{
		Vec2Dragging dragging;

		float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
		ImVec2 buttonSize = { 5, lineHeight };

		ImGui::PushID((label + "Vec3Control").c_str());
		//X
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		if (ImGui::Button("##X", buttonSize))
			values.x = resetValue;
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::PushItemWidth(60);
		dragging.x = ImGui::DragFloat("##Drag X", &values.x, v_speed, v_min, v_max, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		//Y
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		if (ImGui::Button("##Y", buttonSize))
			values.y = resetValue;
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::PushItemWidth(60);
		dragging.y = ImGui::DragFloat("##Drag Y", &values.y, v_speed, v_min, v_max, "%.2f");
		ImGui::PopItemWidth();

		ImGui::PopID();

		return dragging;
	}
}
