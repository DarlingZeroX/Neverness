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

#include "Animation/Core/SpriteAnimationScript.h"

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
