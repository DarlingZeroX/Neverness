#pragma once
#include <string>
#include "../imgui/imgui.h"
#include <NNCore/Interface/HCoreTypes.h>

namespace ImGuiEx
{
	class IMGUI_API RectFilledBase
	{
	public:
		//void SetMinMax(const Vec2& min, const Vec2& max);
		//void SetMinMax(const ImVec2& min, const ImVec2& max);
		void SetRegionAuto(const ImVec2& size, float minX = 0.0f, float maxX = 1.0, float minY = 0.0f, float maxY = 1.0f);
		void SetRegionAutoOffest(const ImVec2& size, float minX = 0.0f, float maxX = 0.0, float minY = 0.0f, float maxY = 0.0f);
		void SetRegionAutoOffest(const ImVec2& min, const ImVec2& size, float minX = 0.0f, float maxX = 0.0, float minY = 0.0f, float maxY = 0.0f);

	protected:
		ImVec2 Min;
		ImVec2 Max;
	};

	class IMGUI_API RectFilledMultiColor : public RectFilledBase
	{
	public:
		void SetColTopLeft(const NN::Core::float4& color);
		void SetColTopRight(const NN::Core::float4& color);
		void SetColBottomLeft(const NN::Core::float4& color);
		void SetColBottomRight(const NN::Core::float4& color);

		void SetCol(const NN::Core::float4& color);
		void SetColLeft(const NN::Core::float4& color);
		void SetColRight(const NN::Core::float4& color);
		void SetColTop(const NN::Core::float4& color);
		void SetColBottom(const NN::Core::float4& color);

		void DebugColor(const std::string& lable = "ColorDebug");
		void WindowDraw();
		void BackgroundDraw();
		void ForegroundDraw();
	private:
		ImVec4 ColTopLeft;
		ImVec4 ColTopRight;
		ImVec4 ColBottomLeft;
		ImVec4 ColBottomRight;
	};

	class IMGUI_API RectFilled : public RectFilledBase
	{
	public:
		void SetCol(const NN::Core::float4& color);

		void DebugColor(const std::string& lable = "ColorDebug");
		void WindowDraw();
		void BackgroundDraw();
		void ForegroundDraw();
	private:
		ImVec2 Min;
		ImVec2 Max;
		ImVec4 Color;
	};
}
