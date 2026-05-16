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

#include "Animation/Audio/AudioAnimation.h"
#include "Scene/Components.h"

namespace NN::Runtime
{
	AudioAnimationState::AudioAnimationState()
	{
		Reset();
	}

	void AudioAnimationState::SetAll(const AnimationData& data)
	{
		volume.currentValue = data.volume;
		visible.currentValue = data.visible ? 1.0f : 0.0f;
	}

	void AudioAnimationState::SetAll(const AudioAnimationState& data)
	{
		volume.currentValue = data.volume.currentValue;
		visible.currentValue = data.visible.currentValue;
	}

	AudioAnimationState::AnimationData AudioAnimationState::GetCurrent() const
	{
		AnimationData data;
		data.volume = volume.currentValue;
		data.visible = visible.currentValue > 0.5f;
		return data;
	}

	void AudioAnimationState::Finish()
	{
		TravelProperty([this](FloatAnimationProperty& property)
			{
				property.Finish();
			});
	}

	bool AudioAnimationState::IsFinish()
	{
		return volume.IsFinish() &&
			visible.IsFinish();
	}

	void AudioAnimationState::Reset()
	{
		SetAll(AnimationData());
		TravelProperty([this](FloatAnimationProperty& property)
			{
				property.active = false;
				property.tween = Tween{};
			});
	}

	void AudioAnimationState::TravelProperty(std::function<void(FloatAnimationProperty& property)> callback)
	{
		callback(volume);
		callback(visible);
	}

	void AudioAnimationScript::TransformVolume(float startTime, float duration, float startVal, float endVal,
		EasingFunction easing)
	{
		state.volume.Start(startVal, endVal, duration, Tween{}, startTime);
	}

	void AudioAnimationScript::TransformVisible(float startTime, float duration, bool startVal, float endVal,
		EasingFunction easing)
	{
		state.visible.Start(startVal ? 1.0f : 0.0f, endVal ? 1.0f : 0.0f, duration, Tween{}, startTime);
	}

	void AudioAnimationScript::Reset()
	{
		state.Reset();
	}

	void AudioAnimationScript::Finish()
	{
		state.Finish();
	}

	void AudioAnimationScript::OnUpdate(NN::Core::HEntityInterface* entity)
	{
		float currentTime = GetCurrentTime(); // 假设存在获取当前时间的函数

		// 更新所有属性
		state.TravelProperty([this, currentTime](FloatAnimationProperty& property)
			{
				property.Update();
			});

		ApplyStateToEntity(entity);
	}

	void AudioAnimationScript::OnFixUpdate(NN::Core::HEntityInterface* entity)
	{

	}

	float AudioAnimationScript::GetCurrentTime() const
	{
		return Core::GetCurrentTime();
	}

	void AudioAnimationScript::ApplyStateToEntity(NN::Core::HEntityInterface* entity)
	{
		auto* audioSource = entity->GetComponent<AudioSourceComponent>();
		if (audioSource)
		{
			if (state.volume.active)
			{
				audioSource->SetVolume(state.volume.currentValue);
				//H_LOG_INFO("state.volume.currentValue %f", state.volume.currentValue);
			}
		}
	}
}
