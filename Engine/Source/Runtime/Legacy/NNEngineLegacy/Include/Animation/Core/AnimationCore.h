/*
* This source file is part of VisionGal, the Visual Novel Engine
*
* For the latest information, see https://darlingzerox.github.io/VisionGalDoc/
* GitHub page: https://github.com/DarlingZeroX/VisionGal
*
* Copyright (c) 2025-present 梦旅缘心
*
* See the LICENSE file in the project root for details.
*/

#pragma once
#include "Tween.h"
#include "NNRuntimeCore/Interface/GameInterface.h"
#include <NNCore/Interface/HVector.h>

namespace NN::Runtime
{
	enum class Animation2DPrimitiveType
	{
		None,
		TranslateX,
		TranslateY,
		ScaleX,
		ScaleY,
		Scale,
		Rotate,
		SpriteAlpha,
		SpriteColor3,
		Visible,
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

		// 用于内部存储起始值
		union
		{
			float __StartValueF;
			float2 __StartValueF2;
			float3 __StartValueF3;
			float4 __StartValueF4;
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
