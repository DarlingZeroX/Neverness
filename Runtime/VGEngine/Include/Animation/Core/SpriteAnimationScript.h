#pragma once
#include "SpriteAnimation.h"

namespace VisionGal
{
    class SpriteFadeInOutTransformScript : public IAnimationScript
    {
    public:
        enum class Direction
        {
	        In,
            Out
        };

		SpriteFadeInOutTransformScript(Direction direction);
        ~SpriteFadeInOutTransformScript() override = default;
		SpriteFadeInOutTransformScript(const SpriteFadeInOutTransformScript&) = delete;
		SpriteFadeInOutTransformScript& operator=(const SpriteFadeInOutTransformScript&) = delete;
		SpriteFadeInOutTransformScript(SpriteFadeInOutTransformScript&&) noexcept = default;
		SpriteFadeInOutTransformScript& operator=(SpriteFadeInOutTransformScript&&) noexcept = default;

        void SetDuration(float duration);
        void SetEasing(EasingFunction easing);
        void Start() override;

        void OnUpdate(Horizon::HEntityInterface* entity) override;
    private:
        Ref<SpriteAnimationScript> m_Script;

        float m_StartOffset, m_EndOffset;
        float m_Duration = 0.f;
        EasingFunction m_EasingFunction = EasingCallbacks::linear;
    };
}
