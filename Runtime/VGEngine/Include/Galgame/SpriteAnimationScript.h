#pragma once
#include "Animation/Core/TransformAnimation.h"

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
