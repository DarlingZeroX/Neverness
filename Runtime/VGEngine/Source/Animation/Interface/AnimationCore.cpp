#include "Animation/Interface/AnimationCore.h"

namespace VisionGal
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
		tween = "";
	}
}


