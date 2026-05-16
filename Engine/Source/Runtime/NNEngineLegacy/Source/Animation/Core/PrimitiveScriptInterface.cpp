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

#include "Animation/Core/PrimitiveScriptInterface.h"

namespace NN::Runtime
{
	bool FloatAnimationPrimitiveScript::ParseLua(sol::object value, Animation2DPrimitive& primitive) const
	{
		if (value.is<double>()) {
			primitive.type = GetPrimitiveType();
			primitive.valueF = value.as<double>();
			return true;
		}
		return false;
	}

	void FloatAnimationPrimitiveScript::OnUpdate(NN::Core::HEntityInterface* entity)
	{
		property.Update();

		if (property.active)
		{
			ApplyValueToEntity(entity ,property.GetCurrentValue());
		}
	}

	bool FloatAnimationPrimitiveScript::IsFinished()
	{
		return property.IsFinish();
	}

	void FloatAnimationPrimitiveScript::StartAnimation(float startValue, float endValue, float duration, Tween tween)
	{
		property.Start(startValue, endValue, duration, tween);
	}

	bool FloatAnimationPrimitiveScript::StartEntityAnimationBase(float startValue, float endValue,Animation2DPrimitive& targetValue, float duration, Tween tween, bool reverse)
	{
		if (reverse)
		{
			endValue = targetValue.__StartValueF;
			tween.reverse();
		}

		targetValue.__StartValueF = startValue;

		property.Start(startValue, endValue, duration, tween);
		return true;
	}

	void Float2AnimationPrimitiveScript::OnUpdate(NN::Core::HEntityInterface* entity)
	{
		property.Update();

		if (property.IsActive())
		{
			ApplyValueToEntity(entity ,property.GetCurrentValue());
		}
	}

	bool Float2AnimationPrimitiveScript::IsFinished()
	{
		return property.IsFinish();
	}

	bool Float2AnimationPrimitiveScript::StartEntityAnimationBase(float2 startValue, float2 endValue,
		Animation2DPrimitive& targetValue, float duration, Tween tween, bool reverse)
	{
		if (reverse)
		{
			endValue = targetValue.__StartValueF2;
			tween.reverse();
		}

		targetValue.__StartValueF2 = startValue;

		property.Start(startValue, endValue, duration, tween);
		return true;
	}

	void Float2AnimationPrimitiveScript::StartAnimation(float2 startValue, float2 endValue, float duration, Tween tween)
	{
		property.Start(startValue, endValue, duration, tween);
	}

	void Float3AnimationPrimitiveScript::OnUpdate(NN::Core::HEntityInterface* entity)
	{
		property.Update();

		if (property.IsActive())
		{
			ApplyValueToEntity(entity ,property.GetCurrentValue());
		}
	}
}
