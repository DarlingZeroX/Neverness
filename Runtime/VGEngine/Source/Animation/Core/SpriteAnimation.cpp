#include "Animation/Core/SpriteAnimation.h"
#include <HCore/Include/Math/HMathHelper.h>

#include "Scene/Components.h"

namespace VisionGal
{
	SpriteAnimationState::SpriteAnimationState()
	{
		Reset();
	}

	void SpriteAnimationState::SetAll(const AnimationData& data)
	{
		alpha.SetCurrentValue(data.alpha);
		color.SetCurrentValue(data.color);
	}

	void SpriteAnimationState::SetAll(const SpriteAnimationState& data)
	{
		alpha.SetCurrentValue(data.alpha);
		color.SetCurrentValue(data.color);
	}

	SpriteAnimationState::AnimationData SpriteAnimationState::GetCurrent() const
	{
		AnimationData data;
		data.alpha = alpha.currentValue;
		data.color = color.GetCurrentValue();
		return data;
	}

	void SpriteAnimationState::Finish()
	{
		TravelProperty([this](IAnimationProperty& property)
			{
				property.Finish();
			});
	}

	bool SpriteAnimationState::IsFinish()
	{
		return alpha.IsFinish() && color.IsFinish();
	}

	void SpriteAnimationState::Reset()
	{
		SetAll(AnimationData());
		TravelProperty([this](IAnimationProperty& property)
			{
				property.Reset();
			});
	}

	void SpriteAnimationState::TravelProperty(std::function<void(IAnimationProperty& property)> callback)
	{
		callback(alpha);
		callback(color);
	}

	void SpriteAnimationScript::TransformAlpha(float startTime, float duration, float startVal, float endVal,
	                                           EasingFunction easing)
	{
		state.alpha.Start(startTime, duration, startVal, endVal, easing);
	}

	void SpriteAnimationScript::Reset()
	{
		state.Reset();
	}

	void SpriteAnimationScript::Finish()
	{
		state.Finish();
	}

	void SpriteAnimationScript::OnUpdate(Horizon::HEntityInterface* entity)
	{
		float currentTime = GetCurrentTime(); // 假设存在获取当前时间的函数

		// 更新所有属性
		state.TravelProperty([this, currentTime](IAnimationProperty& property)
			{
				property.Update(currentTime);
			});

		ApplyStateToEntity(entity);
	}

	void SpriteAnimationScript::OnFixUpdate(Horizon::HEntityInterface* entity)
	{

	}

	float SpriteAnimationScript::GetCurrentTime() const
	{
		return Core::GetCurrentTime();
	}

	void SpriteAnimationScript::ApplyStateToEntity(Horizon::HEntityInterface* entity)
	{
		auto* sr = entity->GetComponent<SpriteRendererComponent>();
		if (sr)
		{
			if (state.alpha.IsFinish() == false)
			{
				sr->color.a = state.alpha.currentValue;
				//H_LOG_INFO("sr->color.a = %f", sr->color.a);
			}
		}
	}
}
