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
#include "AnimationCore.h"
#include "AnimationProperty.h"

namespace VisionGal
{
	struct IAnimationPrimitiveScript: public  IAnimationScript
	{
		~IAnimationPrimitiveScript() override = default;

		virtual bool CanParse(const std::string& key) const = 0;
		virtual bool ParseLua(sol::object value, Animation2DPrimitive& primitive) const = 0;
		virtual Animation2DPrimitiveType GetPrimitiveType() const = 0;
		virtual Ref<IAnimationScript> StartAnimationScript(Horizon::HEntityInterface* entity, const Animation2DProperty& targetProperty, Animation2DPrimitive& primitive, bool reverse) = 0;
	};

	struct FloatAnimationPrimitiveScript  : public IAnimationPrimitiveScript
	{
		FloatAnimationProperty property;

		~FloatAnimationPrimitiveScript() override = default;

		bool ParseLua(sol::object value, Animation2DPrimitive& primitive) const override;
		void OnUpdate(Horizon::HEntityInterface* actor) override;
		bool IsFinished() override;
		bool StartEntityAnimationBase(float startValue, float endValue, Animation2DPrimitive& targetValue, float duration, Tween tween, bool reverse);

		virtual void StartAnimation(float startValue, float endValue, float duration,  Tween tween = Tween{});
		virtual bool StartEntityAnimation(Horizon::HEntityInterface* entity, Animation2DPrimitive& targetValue, float duration, Tween tween, bool reverse) = 0;
		virtual void ApplyValueToEntity(Horizon::HEntityInterface* entity,float value) = 0;
	};

	struct Float2AnimationPrimitiveScript  : public IAnimationPrimitiveScript
	{
		Float2AnimationProperty property;

		~Float2AnimationPrimitiveScript() override = default;

		void OnUpdate(Horizon::HEntityInterface* actor) override;
		bool IsFinished() override;
		bool StartEntityAnimationBase(float2 startValue, float2 endValue, Animation2DPrimitive& targetValue, float duration, Tween tween, bool reverse);

		virtual void StartAnimation(float2 startValue, float2 endValue, float duration,  Tween tween = Tween{});
		virtual bool StartEntityAnimation(Horizon::HEntityInterface* entity, Animation2DPrimitive& targetValue, float duration, Tween tween, bool reverse) = 0;
		virtual void ApplyValueToEntity(Horizon::HEntityInterface* entity, float2 value) = 0;
	};

	struct Float3AnimationPrimitiveScript  : public IAnimationPrimitiveScript
	{
		Float3AnimationProperty property;

		~Float3AnimationPrimitiveScript() override = default;

		void OnUpdate(Horizon::HEntityInterface* actor) override;

		virtual bool StartEntityAnimation(Horizon::HEntityInterface* entity, Animation2DPrimitive& targetValue, float duration, Tween tween, bool reverse) = 0;
		virtual void ApplyValueToEntity(Horizon::HEntityInterface* entity, const float3& value) = 0;
	};

	template<class T>
	struct TFloatAnimationPrimitiveScript  : public FloatAnimationPrimitiveScript
	{
		~TFloatAnimationPrimitiveScript() override = default;

		Ref<IAnimationScript> StartAnimationScript(Horizon::HEntityInterface* entity, const Animation2DProperty& targetProperty, Animation2DPrimitive& primitive, bool reverse) override
		{
			H_ASSERT_NOT_NULL(entity);

			auto script = CreateRef<T>();
			script->StartEntityAnimation(entity, primitive, targetProperty.duration, targetProperty.tween, reverse);

			return script;
		}
	};

	template<class T>
	struct TFloat2AnimationPrimitiveScript  : public Float2AnimationPrimitiveScript
	{
		~TFloat2AnimationPrimitiveScript() override = default;

		Ref<IAnimationScript> StartAnimationScript(Horizon::HEntityInterface* entity, const Animation2DProperty& targetProperty, Animation2DPrimitive& primitive, bool reverse) override
		{
			H_ASSERT_NOT_NULL(entity);

			auto script = CreateRef<T>();
			script->StartEntityAnimation(entity, primitive, targetProperty.duration, targetProperty.tween, reverse);

			return script;
		}
	};
}
