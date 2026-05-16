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
#include "NNRuntimeCore/Interface/GameInterface.h"
#include "../Core/AnimationProperty.h"

namespace NN::Runtime
{
    struct AudioAnimationState {
        AudioAnimationState();
        ~AudioAnimationState() = default;

        struct AnimationData
        {
            float volume = 1.f;
            bool visible = true;
        };

        FloatAnimationProperty volume;
        FloatAnimationProperty visible; // 将bool转为float(0.0/1.0)

        // 立即设置所有属性值
        void SetAll(const AnimationData& data);
        void SetAll(const AudioAnimationState& data);

        void Finish();
        bool IsFinish();
        void Reset();
		 
        void TravelProperty(std::function<void(FloatAnimationProperty& property)> callback);

        // 获取当前状态（转换为原始格式）
        AnimationData GetCurrent() const;
    };

    class AudioAnimationScript : public IAnimationScript
    {
    public:
        AudioAnimationScript() = default;
        ~AudioAnimationScript() override = default;
        AudioAnimationScript(const AudioAnimationScript&) = delete;
        AudioAnimationScript& operator=(const AudioAnimationScript&) = delete;
        AudioAnimationScript(AudioAnimationScript&&) noexcept = default;
        AudioAnimationScript& operator=(AudioAnimationScript&&) noexcept = default;

        // 为单个属性设置动画
        void TransformVolume(float startTime, float duration, float startVal = 1.0f, float endVal = 1.0f, EasingFunction easing = EasingCallbacks::linear);
        void TransformVisible(float startTime, float duration, bool startVal = true, float endVal = true, EasingFunction easing = EasingCallbacks::linear);

        void Reset();
        void Finish();

        void OnUpdate(NN::Core::HEntityInterface* entity) override;
        void OnFixUpdate(NN::Core::HEntityInterface* entity) override;

    private:
        void ApplyStateToEntity(NN::Core::HEntityInterface* entity);
        float GetCurrentTime() const;

        AudioAnimationState state;
    };


}
