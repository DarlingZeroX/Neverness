#pragma once
#include "AudioAnimation.h"

namespace VisionGal
{
    class AudioFadeInOutAnimationScript : public IAnimationScript
    {
    public:
        enum class Direction
        {
            In,
            Out
        };

        AudioFadeInOutAnimationScript(Direction direction);
        ~AudioFadeInOutAnimationScript() override = default;
        AudioFadeInOutAnimationScript(const AudioFadeInOutAnimationScript&) = delete;
        AudioFadeInOutAnimationScript& operator=(const AudioFadeInOutAnimationScript&) = delete;
        AudioFadeInOutAnimationScript(AudioFadeInOutAnimationScript&&) noexcept = default;
        AudioFadeInOutAnimationScript& operator=(AudioFadeInOutAnimationScript&&) noexcept = default;

        void SetDuration(float duration);
        void SetEasing(EasingFunction easing);
		void SetInVolume(float volume) { m_EndVolume = volume; }
		void SetOutVolume(float volume) { m_StartVolume = volume; }
        void Start() override;

        void OnUpdate(Horizon::HEntityInterface* entity) override;
    private:
        Ref<AudioAnimationScript> m_Script;

        Direction m_Direction = Direction::In;
        float m_StartVolume, m_EndVolume;
        float m_Duration = 0.f;
        EasingFunction m_EasingFunction = EasingCallbacks::linear;
    };
}
