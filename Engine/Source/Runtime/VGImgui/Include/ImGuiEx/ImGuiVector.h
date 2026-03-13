#pragma once
#include <HCore/Include/Math/HVector.h>
#include <string>
#include "../imgui/imgui.h"
#include <memory>

namespace ImGuiEx
{
	struct Vec3Dragging
	{
		bool x, y, z;

		Vec3Dragging()
			:x(false), y(false), z(false)
		{}
	};

	IMGUI_API Vec3Dragging DrawVec3Control(
		const std::string& label,
		Horizon::float3& values,
		float resetValue,
		float v_speed = 1.0f,
		float v_min = 0.0f, float v_max = 0.0f
	);

	struct Vec2Dragging
	{
		bool x, y;

		Vec2Dragging()
			:x(false), y(false)
		{
		}
	};

	IMGUI_API Vec2Dragging DrawVec2Control(
		const std::string& label,
		Horizon::float2& values,
		float resetValue,
		float v_speed = 1.0f,
		float v_min = 0.0f, float v_max = 0.0f
	);
}