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
#include "../../Interface/GameInterface.h"
#include "Tween.h"

namespace VisionGal
{
    // 缓动函数类型
    using EasingFunction = std::function<float(float)>;

	struct IAnimationProperty
	{
		virtual ~IAnimationProperty() = default;

		// 更新属性值
		virtual void Update(float currentTime) = 0;
		virtual void Finish() = 0;
		virtual bool IsFinish() = 0;
		virtual void Reset() = 0;
	};

	// 单个Float的插值器
	struct FloatAnimationProperty : public IAnimationProperty{
		float startValue = 0.0f;
		float endValue = 0.0f;
		float currentValue = 0.0f;
		float startTime = 0.0f;
		float duration = 1.0f;
		Tween tween = Tween{}; // 默认线性插值
		bool active = false;
		bool isFinish = false;

		~FloatAnimationProperty() override = default;

		void SetCurrentValue(float value);
		void SetCurrentValue(const FloatAnimationProperty& value);
		float GetCurrentValue();

		// 更新属性值
		void Update(float currentTime) override;

		// 开始插值
		void Start(float startTime, float duration, float startValue, float endValue, Tween tween = Tween{});

		void Finish() override;
		bool IsFinish() override;
		void Reset() override;
	};

	// Float2插值属性
	struct Float2AnimationProperty : public IAnimationProperty {
		FloatAnimationProperty value0;
		FloatAnimationProperty value1;

		~Float2AnimationProperty() override = default;

		void SetCurrentValue(const float2& value);
		void SetCurrentValue(const Float2AnimationProperty& value);
		float2 GetCurrentValue() const;

		// 更新属性值
		void Update(float currentTime) override;

		// 开始插值
		void Start(float startTime, float duration, const float2& startValue, const float2& endValue, Tween tween = Tween{});

		void Finish() override;
		bool IsFinish() override;
		void Reset() override;
	};

	// Float3插值属性
	struct Float3AnimationProperty : public IAnimationProperty {
		FloatAnimationProperty value0;
		Float2AnimationProperty value12;

		~Float3AnimationProperty() override = default;

		void SetCurrentValue(const float3& value);
		void SetCurrentValue(const Float3AnimationProperty& value);
		float3 GetCurrentValue() const;

		// 更新属性值
		void Update(float currentTime) override;

		// 开始插值
		void Start(float startTime, float duration, const float3& startValue, const float3& endValue, Tween tween = Tween{});

		void Finish() override;
		bool IsFinish() override;
		void Reset() override;
	};

	// Float4插值属性
	struct Float4AnimationProperty : public IAnimationProperty {
		Float2AnimationProperty value01;
		Float2AnimationProperty value23;

		~Float4AnimationProperty() override = default;

		void SetCurrentValue(const float4& value);
		void SetCurrentValue(const Float4AnimationProperty& value);
		float4 GetCurrentValue() const;

		// 更新属性值
		void Update(float currentTime) override;

		// 开始插值
		void Start(float startTime, float duration, const float4& startValue, const float4& endValue, Tween tween = Tween{});

		void Finish() override;
		bool IsFinish() override;
		void Reset() override;
	};

    struct EasingCallbacks
    {
        static EasingFunction linear;
    };

	struct FloatAnimationPropertyScript : public  IAnimationScript
	{
		~FloatAnimationPropertyScript() override = default;

		FloatAnimationProperty property;

		bool IsFinished() override;
	protected:
		void Start(float duration, float startVal = 1.0f, float endVal = 1.0f, Tween tween = Tween{});

		float GetCurrentTime();

		void UpdateProperty();
	};
}
