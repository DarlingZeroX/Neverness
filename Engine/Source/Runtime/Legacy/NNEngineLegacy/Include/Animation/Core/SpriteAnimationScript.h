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
//#include "SpriteAnimation.h"
#include "PrimitiveScript.h"

namespace NN::Runtime
{
    class VG_ENGINE_API SpriteFadeInOutTransformScript : public IAnimationScript
    {
    public:
        enum class Direction
        {
	        In,
            Out
        };

		SpriteFadeInOutTransformScript(NN::Core::HEntityInterface* entity, Direction direction);
        ~SpriteFadeInOutTransformScript() override = default;
		SpriteFadeInOutTransformScript(const SpriteFadeInOutTransformScript&) = delete;
		SpriteFadeInOutTransformScript& operator=(const SpriteFadeInOutTransformScript&) = delete;
		SpriteFadeInOutTransformScript(SpriteFadeInOutTransformScript&&) noexcept = default;
		SpriteFadeInOutTransformScript& operator=(SpriteFadeInOutTransformScript&&) noexcept = default;

        void SetDuration(float duration);
        void SetEasing(EasingFunction easing);
        void Start() override;

        void OnUpdate(NN::Core::HEntityInterface* entity) override;
    private:
        Ref<SpriteAlphaAnimationScript> m_Script;

        float m_StartOffset, m_EndOffset;
        float m_Duration = 0.f;
        EasingFunction m_EasingFunction = EasingCallbacks::linear;
    };
}
