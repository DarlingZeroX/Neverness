#pragma once
#include "Tween.h"
#include "../../Interface/GameInterface.h"

namespace VisionGal
{
	enum class Animation2DPrimitiveType
	{
		None,
		TranslateX,
		TranslateY,
		ScaleX,
		ScaleY,
		Rotate,
		Alpha
	};

	struct Animation2DPrimitive
	{
		Animation2DPrimitiveType type = Animation2DPrimitiveType::None;

		union
		{
			float valueF;
			float2 valueF2;
			float3 valueF3;
			float4 valueF4;
		};

		Animation2DPrimitive();
	};

	struct Animation2DProperty
	{
		std::vector<Animation2DPrimitive> primitive;

		float duration = 0.f;
		Tween tween = Tween{};

		void Reset();
	};
}
