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
 
#include "Galgame/SpriteAnimationScript.h"
#include "Galgame/GameEngineCore.h"

namespace VisionGal::GalGame
{
	ScrollTransformScript::ScrollTransformScript(Direction direction, float2 texSize)
	{
		m_Script = CreateRef<TransformAnimationScript>();

		switch (direction)
		{
		case Direction::Left:
			m_StartOffset = -GameEngineCore::GetSpriteXOffset(texSize.x);
			m_EndOffset = -m_StartOffset;
			m_IsHorizontal = true;
			break;
		case Direction::Right:
			m_StartOffset = GameEngineCore::GetSpriteXOffset(texSize.x);
			m_EndOffset = -m_StartOffset;
			m_IsHorizontal = true;
			break;
		case Direction::Up:
			m_StartOffset = GameEngineCore::GetSpriteYOffset(texSize.y);
			m_EndOffset = -m_StartOffset;
			m_IsHorizontal = false;
			break;
		case Direction::Down:
			m_StartOffset = -GameEngineCore::GetSpriteYOffset(texSize.y);
			m_EndOffset = -m_StartOffset;
			m_IsHorizontal = false;
			break;
		}

	}

	void ScrollTransformScript::SetDuration(float duration)
	{
		m_Duration = duration;
	}

	void ScrollTransformScript::SetEasing(EasingFunction easing)
	{
		m_EasingFunction = easing;
	}

	void ScrollTransformScript::Start()
	{
		if (m_IsHorizontal)
		{
			m_Script->TransformXOffset(
				Core::GetCurrentTime(),
				m_Duration,
				m_StartOffset,
				m_EndOffset,
				m_EasingFunction
			);
		}
		else
		{
			m_Script->TransformYOffset(
				Core::GetCurrentTime(), 
				m_Duration,
				m_StartOffset,
				m_EndOffset, 
				m_EasingFunction
			);
		}
	}

	void ScrollTransformScript::OnUpdate(Horizon::HEntityInterface* entity)
	{
		m_Script->OnUpdate(entity);
	}
}
