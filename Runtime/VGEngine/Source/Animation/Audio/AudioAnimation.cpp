#include "Animation/Audio/AudioAnimation.h"
#include "Scene/Components.h"

namespace VisionGal
{
	AudioAnimationState::AudioAnimationState()
	{
		Reset();
	}

	void AudioAnimationState::SetAll(const TransformData& data)
	{
		volume.currentValue = data.volume;
		visible.currentValue = data.visible ? 1.0f : 0.0f;
	}

	void AudioAnimationState::SetAll(const AudioAnimationState& data)
	{
		volume.currentValue = data.volume.currentValue;
		visible.currentValue = data.visible.currentValue;
	}

	AudioAnimationState::TransformData AudioAnimationState::GetCurrent() const
	{
		TransformData data;
		data.volume = volume.currentValue;
		data.visible = visible.currentValue > 0.5f;
		return data;
	}

	void AudioAnimationState::Finish()
	{
		TravelProperty([this](SingleAnimationProperty& property)
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
		SetAll(TransformData());
		TravelProperty([this](SingleAnimationProperty& property)
			{
				property.active = false;
				property.easing = EasingCallbacks::linear;
			});
	}

	void AudioAnimationState::TravelProperty(std::function<void(SingleAnimationProperty& property)> callback)
	{
		callback(volume);
		callback(visible);
	}

	void AudioAnimationScript::TransformVolume(float startTime, float duration, float startVal, float endVal,
		EasingFunction easing)
	{
		state.volume.Start(startTime, duration, startVal, endVal, easing);
	}

	void AudioAnimationScript::TransformVisible(float startTime, float duration, bool startVal, float endVal,
		EasingFunction easing)
	{
		state.visible.Start(startTime, duration, startVal ? 1.0f : 0.0f, endVal ? 1.0f : 0.0f, easing);
	}

	void AudioAnimationScript::Reset()
	{
		state.Reset();
	}

	void AudioAnimationScript::Finish()
	{
		state.Finish();
	}

	void AudioAnimationScript::OnUpdate(Horizon::HEntityInterface* entity)
	{
		float currentTime = GetCurrentTime(); // 假设存在获取当前时间的函数

		// 更新所有属性
		state.TravelProperty([this, currentTime](SingleAnimationProperty& property)
			{
				property.Update(currentTime);
			});

		ApplyStateToEntity(entity);
	}

	void AudioAnimationScript::OnFixUpdate(Horizon::HEntityInterface* entity)
	{

	}

	float AudioAnimationScript::GetCurrentTime() const
	{
		return Core::GetCurrentTime();
	}

	void AudioAnimationScript::ApplyStateToEntity(Horizon::HEntityInterface* entity)
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
