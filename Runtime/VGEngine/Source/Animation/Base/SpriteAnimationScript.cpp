#include "Animation/Base/SpriteAnimationScript.h"

namespace VisionGal
{
	SpriteFadeInOutTransformScript::SpriteFadeInOutTransformScript(Direction direction)
	{
		m_Script = CreateRef<SpriteAnimationScript>();

		switch (direction)
		{
		case Direction::In:
			m_StartOffset = 0.0f;
			m_EndOffset = 1.0f;
			break;
		case Direction::Out:
			m_StartOffset = 1.0f;
			m_EndOffset = 0.0f;
			break;
		}
	}

	void SpriteFadeInOutTransformScript::SetDuration(float duration)
	{
		m_Duration = duration;
	}

	void SpriteFadeInOutTransformScript::SetEasing(EasingFunction easing)
	{
		m_EasingFunction = easing;
	}

	void SpriteFadeInOutTransformScript::Start()
	{
		m_Script->TransformAlpha(
			Core::GetCurrentTime(),
			m_Duration,
			m_StartOffset,
			m_EndOffset,
			m_EasingFunction
		);
	}

	void SpriteFadeInOutTransformScript::OnUpdate(Horizon::HEntityInterface* entity)
	{
		m_Script->OnUpdate(entity);
	}
}
