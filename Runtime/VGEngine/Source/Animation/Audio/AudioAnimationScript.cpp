#include "Animation/Audio/AudioAnimationScript.h"

namespace VisionGal
{
	AudioFadeInOutAnimationScript::AudioFadeInOutAnimationScript(Direction direction)
	{
		m_Script = CreateRef<AudioAnimationScript>();

		switch (direction)
		{
		case Direction::In:
			m_StartVolume = 0.0f;
			m_EndVolume = 1.0f;
			break;
		case Direction::Out:
			m_StartVolume = 1.0f;
			m_EndVolume = 0.0f;
			break;
		}
	}

	void AudioFadeInOutAnimationScript::SetDuration(float duration)
	{
		m_Duration = duration;
	}

	void AudioFadeInOutAnimationScript::SetEasing(EasingFunction easing)
	{
		m_EasingFunction = easing;
	}

	void AudioFadeInOutAnimationScript::Start()
	{
		m_Script->TransformVolume(
			Core::GetCurrentTime(),
			m_Duration,
			m_StartVolume,
			m_EndVolume,
			m_EasingFunction
		);
	}

	void AudioFadeInOutAnimationScript::OnUpdate(Horizon::HEntityInterface* entity)
	{
		m_Script->OnUpdate(entity);
	}
}
