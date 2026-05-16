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

#include "Animation/Core/AnimationCore.h"

namespace NN::Runtime
{
	Animation2DPrimitive::Animation2DPrimitive()
	{
		valueF = 0;
		valueF2 = float2(0);
		valueF3 = float3(0);
		valueF4 = float4(0);
	}

	void Animation2DProperty::Reset()
	{
		primitive.clear();
		duration = 0.f;
		tween = Tween{};
	}
}


