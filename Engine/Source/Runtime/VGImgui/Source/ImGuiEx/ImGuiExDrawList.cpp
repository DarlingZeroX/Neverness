#include "pch.h"
#include "ImGuiEx/ImGuiExDrawList.h"
using namespace ImGui;

namespace ImGuiEx
{
	static void FillImVec2(const Horizon::float4& lhs, ImVec2& rhs)
	{
		rhs.x = lhs.x;
		rhs.y = lhs.y;
	}

	static void FillImVec4(const Horizon::float4& lhs, ImVec4& rhs)
	{
		rhs.x = lhs.x;
		rhs.y = lhs.y;
		rhs.z = lhs.z;
		rhs.w = lhs.w;
	}

	//void RectFilledMultiColor::SetMinMax(const Vec2& min, const Vec2& max)
	//{
	//	FillImVec2(min, Min);
	//	FillImVec2(max, Max);
	//}
	//
	//void RectFilledMultiColor::SetMinMax(const ImVec2& min, const ImVec2& max)
	//{
	//	Min.x = min.x;
	//	Min.y = max.y;
	//	Max.x = max.x;
	//	Max.y = max.x;
	//}

	void RectFilledBase::SetRegionAuto(const ImVec2& size, float minX, float maxX, float minY, float maxY)
	{
		Min = ImGui::GetCursorScreenPos();

		Max.x = Min.x + maxX * size.x;
		Max.y = Min.y + maxY * size.y;

		Min.x += minX * size.x;
		Min.y += minY * size.y;
	}

	void RectFilledBase::SetRegionAutoOffest(const ImVec2& size, float minX, float maxX, float minY, float maxY)
	{
		Min = ImGui::GetCursorScreenPos();

		Max.x = Min.x + maxX + size.x;
		Max.y = Min.y + maxY + size.y;

		Min.x += minX;
		Min.y += minY;
	}

	void RectFilledBase::SetRegionAutoOffest(const ImVec2& min, const ImVec2& size, float minX, float maxX, float minY, float maxY)
	{
		Min = min;

		Max.x = min.x + maxX + size.x;
		Max.y = min.y + maxY + size.y;

		Min.x += minX;
		Min.y += minY;
	}

	void RectFilledMultiColor::SetColTopLeft(const Horizon::float4& color)
	{
		FillImVec4(color, ColTopLeft);
	}

	void RectFilledMultiColor::SetColTopRight(const Horizon::float4& color)
	{
		FillImVec4(color, ColTopRight);
	}

	void RectFilledMultiColor::SetColBottomLeft(const Horizon::float4& color)
	{
		FillImVec4(color, ColBottomLeft);
	}

	void RectFilledMultiColor::SetColBottomRight(const Horizon::float4& color)
	{
		FillImVec4(color, ColBottomRight);
	}

	void RectFilledMultiColor::SetCol(const Horizon::float4& color)
	{
		SetColLeft(color);
		SetColRight(color);
	}

	void RectFilledMultiColor::SetColLeft(const Horizon::float4& color)
	{
		SetColTopLeft(color);
		SetColBottomLeft(color);
	}

	void RectFilledMultiColor::SetColRight(const Horizon::float4& color)
	{
		SetColTopRight(color);
		SetColBottomRight(color);
	}

	void RectFilledMultiColor::SetColTop(const Horizon::float4& color)
	{
		SetColTopLeft(color);
		SetColTopRight(color);
	}

	void RectFilledMultiColor::SetColBottom(const Horizon::float4& color)
	{
		SetColBottomLeft(color);
		SetColBottomRight(color);
	}

	void RectFilledMultiColor::DebugColor(const std::string& lable)
	{
		if (!ImGui::Begin(lable.c_str()))
		{
			ImGui::End();
			return;
		}

		ImGui::ColorEdit4("ColTopLeft", &ColTopLeft.x);
		ImGui::ColorEdit4("ColTopRight", &ColTopRight.x);
		ImGui::ColorEdit4("ColBottomLeft", &ColBottomLeft.x);
		ImGui::ColorEdit4("ColBottomRight", &ColBottomRight.x);

		static bool setAllColor = false;
		ImGui::Checkbox("SetAllColor", &setAllColor);
		if (setAllColor)
		{
			ImGui::ColorEdit4("Color", &ColTopLeft.x);
			ColTopRight = ColTopLeft;
			ColBottomLeft = ColTopLeft;
			ColBottomRight = ColTopLeft;
		}

		static bool setColorLeftRight = false;
		ImGui::Checkbox("SetColorLeftRight", &setColorLeftRight);
		if (setColorLeftRight)
		{
			ImGui::ColorEdit4("ColorLeft", &ColTopLeft.x);
			ColBottomLeft = ColTopLeft;
			ImGui::ColorEdit4("ColorRight", &ColTopRight.x);
			ColBottomRight = ColTopRight;
		}

		static bool setColorTopBottom = false;
		ImGui::Checkbox("SetColorTopBottom", &setColorTopBottom);
		if (setColorTopBottom)
		{
			ImGui::ColorEdit4("ColorTop", &ColTopLeft.x);
			ColTopRight = ColTopLeft;
			ImGui::ColorEdit4("ColorBottom", &ColBottomLeft.x);
			ColBottomRight = ColBottomLeft;
		}

		ImGui::End();
	}

	void RectFilledMultiColor::WindowDraw()
	{
		ImGui::GetWindowDrawList()->AddRectFilledMultiColor(Min, Max,
			ImGui::ColorConvertFloat4ToU32(ColTopLeft),
			ImGui::ColorConvertFloat4ToU32(ColTopRight),
			ImGui::ColorConvertFloat4ToU32(ColBottomRight),
			ImGui::ColorConvertFloat4ToU32(ColBottomLeft)
		);
	}
	void RectFilledMultiColor::BackgroundDraw()
	{
		ImGui::GetBackgroundDrawList()->AddRectFilledMultiColor(Min, Max,
			ImGui::ColorConvertFloat4ToU32(ColTopLeft),
			ImGui::ColorConvertFloat4ToU32(ColTopRight),
			ImGui::ColorConvertFloat4ToU32(ColBottomRight),
			ImGui::ColorConvertFloat4ToU32(ColBottomLeft)
		);
	}

	void RectFilledMultiColor::ForegroundDraw()
	{
		ImGui::GetForegroundDrawList()->AddRectFilledMultiColor(Min, Max,
			ImGui::ColorConvertFloat4ToU32(ColTopLeft),
			ImGui::ColorConvertFloat4ToU32(ColTopRight),
			ImGui::ColorConvertFloat4ToU32(ColBottomRight),
			ImGui::ColorConvertFloat4ToU32(ColBottomLeft)
		);
	}

	void RectFilled::SetCol(const Horizon::Vec4& color)
	{
		FillImVec4(color, Color);
	}

	void RectFilled::DebugColor(const std::string& lable)
	{
		ImGui::ColorEdit4("Color", &Color.x);
	}

	void RectFilled::WindowDraw()
	{
		ImGui::GetWindowDrawList()->AddRectFilled(Min, Max, ImGui::ColorConvertFloat4ToU32(Color), 1.0f);
	}

	void RectFilled::BackgroundDraw()
	{
		ImGui::GetBackgroundDrawList()->AddRectFilled(Min, Max, ImGui::ColorConvertFloat4ToU32(Color), 1.0f);
	}

	void RectFilled::ForegroundDraw()
	{
		ImGui::GetForegroundDrawList()->AddRectFilled(Min, Max, ImGui::ColorConvertFloat4ToU32(Color), 1.0f);
	}
}