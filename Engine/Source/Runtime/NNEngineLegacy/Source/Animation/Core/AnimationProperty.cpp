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

#include "Animation/Core/AnimationProperty.h"
#include <NNKernel/Include/Math/HMathHelper.h>
#include <NNKernel/Interface/HVector.h>

namespace VisionGal
{
	EasingFunction EasingCallbacks::linear = [](float t) { return t; };

	float IAnimationProperty::GetCurrentTime()
	{
		return Core::GetCurrentTime();
	}

	void FloatAnimationProperty::SetCurrentValue(float value)
	{
		currentValue = value;
	}

	void FloatAnimationProperty::SetCurrentValue(const FloatAnimationProperty& value)
	{
		currentValue = value.currentValue;
	}

	float FloatAnimationProperty::GetCurrentValue()
	{
		return currentValue;
	}

	void FloatAnimationProperty::Update()
	{
		float currentTime = GetCurrentTime();

		if (active == false)
		{
			return;
		}

		if (isFinish == true)
		{
			active = false;
			return;
		}

		if (currentTime < startTime)
		{
			isFinish = true;
			return;
		}

		float elapsed = currentTime - startTime;
		if (elapsed >= duration) {
			currentValue = endValue;
			isFinish = true;
		}
		else {
			float t = elapsed / duration;
			currentValue = startValue + tween(t) * (endValue - startValue);
		}

		// �ж��Ƿ����
		//if (startValue < endValue)
		//{
		//	if (currentValue >= endValue)
		//	{
		//		currentValue = endValue;
		//		isFinish = true;
		//	}
		//}
		//else
		//{
		//	if (currentValue <= endValue)
		//	{
		//		currentValue = endValue;
		//		isFinish = true;
		//	}
		//}
	}

	void FloatAnimationProperty::Start(float startValue, float endValue, float duration, Tween tween, float startTime)
	{
		if (startTime == 0.f)
			startTime = GetCurrentTime();

		this->startTime = startTime;
		this->duration = duration;
		this->startValue = startValue;
		this->endValue = endValue;
		this->tween = tween;
		this->active = true;
		this->isFinish = false;
	}

	void FloatAnimationProperty::Finish()
	{
		currentValue = endValue;
	}

	bool FloatAnimationProperty::IsFinish()
	{
		return active == false;
	}

	bool FloatAnimationProperty::IsActive()
	{
		return active;
	}

	void FloatAnimationProperty::Reset()
	{
		active = false;
	}

	void Float2AnimationProperty::SetCurrentValue(const float2& value)
	{
		value0.currentValue = value.x;
		value1.currentValue = value.y;
	}

	void Float2AnimationProperty::SetCurrentValue(const Float2AnimationProperty& value)
	{
		value0.SetCurrentValue(value.value0);
		value1.SetCurrentValue(value.value1);
	}

	float2 Float2AnimationProperty::GetCurrentValue() const
	{
		return float2(value0.currentValue, value1.currentValue);
	}

	void Float2AnimationProperty::Update()
	{
		value0.Update();
		value1.Update();
	}

	void Float2AnimationProperty::Start(const float2& startValue, const float2& endValue, float duration, Tween tween, float startTime)
	{
		value0.Start(startValue.x, endValue.x, duration, tween, startTime);
		value1.Start(startValue.x, endValue.x, duration, tween, startTime);
	}

	void Float2AnimationProperty::Finish()
	{
		value0.Finish();
		value1.Finish();
	}

	bool Float2AnimationProperty::IsFinish()
	{
		return value0.IsFinish() && value1.IsFinish();
	}

	bool Float2AnimationProperty::IsActive()
	{
		return value0.IsActive() || value1.IsActive();
	}

	void Float2AnimationProperty::Reset()
	{
		value0.Reset();
		value1.Reset();
	}

	void Float3AnimationProperty::SetCurrentValue(const float3& value)
	{
		value0.currentValue = value.x;
		value12.SetCurrentValue(float2(value.y, value.z));
	}

	void Float3AnimationProperty::SetCurrentValue(const Float3AnimationProperty& value)
	{
		value0.SetCurrentValue(value.value0);
		value12.SetCurrentValue(value.value12);
	}

	float3 Float3AnimationProperty::GetCurrentValue() const
	{
		return float3(value0.currentValue, value12.value0.currentValue, value12.value1.currentValue);
	}

	void Float3AnimationProperty::Update()
	{
		value0.Update();
		value12.Update();
	}

	void Float3AnimationProperty::Start(const float3& startValue, const float3& endValue, float duration, Tween tween, float startTime)
	{
		value0.Start(startValue.x, endValue.x, duration, tween, startTime);
		value12.Start(float2(startValue.y, startValue.z), float2(endValue.y, endValue.z), duration, tween, startTime);
	}

	void Float3AnimationProperty::Finish()
	{
		value0.Finish();
		value12.Finish();
	}

	bool Float3AnimationProperty::IsFinish()
	{
		return value0.IsFinish() && value12.IsFinish();
	}

	bool Float3AnimationProperty::IsActive()
	{
		return value0.IsActive() || value12.IsActive();
	}

	void Float3AnimationProperty::Reset()
	{
		value0.Reset();
		value12.Reset();
	}

	void Float4AnimationProperty::SetCurrentValue(const float4& value)
	{
		value01.SetCurrentValue(float2(value.x, value.y));
		value23.SetCurrentValue(float2(value.z, value.w));
	}

	void Float4AnimationProperty::SetCurrentValue(const Float4AnimationProperty& value)
	{
		value01.SetCurrentValue(value.value01);
		value23.SetCurrentValue(value.value23);
	}

	float4 Float4AnimationProperty::GetCurrentValue() const
	{
		return float4(value01.value0.currentValue, 
			value01.value1.currentValue,
			value23.value0.currentValue, 
			value23.value1.currentValue
		);
	}

	void Float4AnimationProperty::Update()
	{
		value01.Update();
		value23.Update();
	}

	void Float4AnimationProperty::Start(const float4& startValue, const float4& endValue, float duration, Tween tween, float startTime)
	{
		value01.Start(float2(startValue.x, startValue.y), float2(endValue.x, endValue.y), duration, tween, startTime);
		value23.Start(float2(startValue.z, startValue.w), float2(endValue.z, endValue.w), duration, tween, startTime);
	}

	void Float4AnimationProperty::Finish()
	{
		value01.Finish();
		value23.Finish();
	}

	bool Float4AnimationProperty::IsFinish()
	{
		return value01.IsFinish() && value23.IsFinish();
	}

	bool Float4AnimationProperty::IsActive()
	{
		return value01.IsActive() || value23.IsActive();
	}

	void Float4AnimationProperty::Reset()
	{
		value01.Reset();
		value23.Reset();
	}
}
