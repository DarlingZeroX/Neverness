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
#include "../../Interface/GameInterface.h"
#include "AnimationProperty.h"

namespace VisionGal
{
	struct SpriteAnimationState {
        SpriteAnimationState();

        struct AnimationData
        {
            float alpha = 1.f;
			float3 color = float3(1.f, 1.f, 1.f);
        };

        FloatAnimationProperty alpha;
		Float3AnimationProperty color;

        // 立即设置所有属性值
        void SetAll(const AnimationData& data);
        void SetAll(const SpriteAnimationState& data);

        void Finish();
        bool IsFinish();
        void Reset();
		 
        void TravelProperty(std::function<void(IAnimationProperty& property)> callback);

        // 获取当前状态（转换为原始格式）
        AnimationData GetCurrent() const;
    };

    class SpriteAnimationScript: public IAnimationScript
    {
    public:
        SpriteAnimationScript() = default;
        ~SpriteAnimationScript() override = default;
        SpriteAnimationScript(const SpriteAnimationScript&) = delete;
        SpriteAnimationScript& operator=(const SpriteAnimationScript&) = delete;
        SpriteAnimationScript(SpriteAnimationScript&&) noexcept = default;
        SpriteAnimationScript& operator=(SpriteAnimationScript&&) noexcept = default;

        // 为单个属性设置动画
        void TransformAlpha(float startTime, float duration, float startVal = 1.0f, float endVal = 1.0f, EasingFunction easing = EasingCallbacks::linear);

        void Reset();
        void Finish();

        void OnUpdate(Horizon::HEntityInterface* entity) override;
        void OnFixUpdate(Horizon::HEntityInterface* entity) override;

    private:
        void ApplyStateToEntity(Horizon::HEntityInterface* entity);
        float GetCurrentTime() const;

        SpriteAnimationState state;
    };


}
