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
#include "VGEngine/Include/Animation/Core/TransformAnimation.h"
 //#include "Animation/Core/PrimitiveScript.h"

namespace VisionGal::GalGame
{
    class ScrollTransformScript : public IAnimationScript
    {
    public:
        enum class Direction
        {
            Left,
            Right,
            Up,
            Down
        };

		ScrollTransformScript(Direction direction, float2 texSize);
        ~ScrollTransformScript() override = default;
		ScrollTransformScript(const ScrollTransformScript&) = delete;
		ScrollTransformScript& operator=(const ScrollTransformScript&) = delete;
		ScrollTransformScript(ScrollTransformScript&&) noexcept = default;
		ScrollTransformScript& operator=(ScrollTransformScript&&) noexcept = default;

        void SetDuration(float duration);
        void SetEasing(EasingFunction easing);
        void Start() override;

        void OnUpdate(Horizon::HEntityInterface* entity) override;
    private:
        Ref<TransformAnimationScript> m_Script;
        bool m_IsHorizontal = true;
        float m_StartOffset, m_EndOffset;

        float m_Duration = 0.f;
        EasingFunction m_EasingFunction = EasingCallbacks::linear;
    };
}
