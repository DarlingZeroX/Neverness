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

#include "Animation/Audio/AudioAnimationScript.h"

namespace NN::Runtime
{
	AudioFadeInOutAnimationScript::AudioFadeInOutAnimationScript(Direction direction)
	{
		m_Script = MakeRef<AudioAnimationScript>();

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
			RuntimeCore::GetCurrentTime(),
			m_Duration,
			m_StartVolume,
			m_EndVolume,
			m_EasingFunction
		);
	}

	void AudioFadeInOutAnimationScript::OnUpdate(NN::Core::HEntityInterface* entity)
	{
		m_Script->OnUpdate(entity);
	}
}
