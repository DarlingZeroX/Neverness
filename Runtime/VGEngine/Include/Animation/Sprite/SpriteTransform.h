#pragma once
#include "../../Interface/GameInterface.h"
#include "../Interface/AnimationProperty.h"

namespace VisionGal
{
	struct SpriteTransformState {
        SpriteTransformState();

        struct TransformData
        {
            float alpha = 1.f;
            float xoffset = 0.f;
            float yoffset = 0.f;
            float rotate = 0.f;
            float zoom = 1.f;
            float xzoom = 1.f;
            float yzoom = 1.f;
            bool visible = true;
        };

        SingleAnimationProperty alpha;
        SingleAnimationProperty xoffset;
        SingleAnimationProperty yoffset; 
        SingleAnimationProperty rotate;
        SingleAnimationProperty zoom;
        SingleAnimationProperty xzoom;
        SingleAnimationProperty yzoom;
        SingleAnimationProperty visible; // 将bool转为float(0.0/1.0)

        // 立即设置所有属性值
        void SetAll(const TransformData& data);
        void SetAll(const SpriteTransformState& data);

        void Finish();
        bool IsFinish();
        void Reset();
		 
        void TravelProperty(std::function<void(SingleAnimationProperty& property)> callback);

        // 获取当前状态（转换为原始格式）
        TransformData GetCurrent() const;
    };

    class SpriteTransformScript: public IAnimationScript
    {
    public:
        SpriteTransformScript() = default;
        ~SpriteTransformScript() override = default;
        SpriteTransformScript(const SpriteTransformScript&) = delete;
        SpriteTransformScript& operator=(const SpriteTransformScript&) = delete;
        SpriteTransformScript(SpriteTransformScript&&) noexcept = default;
        SpriteTransformScript& operator=(SpriteTransformScript&&) noexcept = default;

        // 为单个属性设置动画
        void TransformAlpha(float startTime, float duration, float startVal = 1.0f, float endVal = 1.0f, EasingFunction easing = EasingCallbacks::linear);
        void TransformXOffset(float startTime, float duration, float startVal = 0.0f, float endVal = 0.0f, EasingFunction easing = EasingCallbacks::linear);
        void TransformYOffset(float startTime, float duration, float startVal = 0.0f, float endVal = 0.0f, EasingFunction easing = EasingCallbacks::linear);
        void TransformRotate(float startTime, float duration, float startVal = 0.0f, float endVal = 0.0f, EasingFunction easing = EasingCallbacks::linear);
        void TransformZoom(float startTime, float duration, float startVal = 1.0f, float endVal = 1.0f, EasingFunction easing = EasingCallbacks::linear);
        void TransformXZoom(float startTime, float duration, float startVal = 1.0f, float endVal = 1.0f, EasingFunction easing = EasingCallbacks::linear);
        void TransformYZoom(float startTime, float duration, float startVal = 1.0f, float endVal = 1.0f, EasingFunction easing = EasingCallbacks::linear);
        void TransformVisible(float startTime, float duration, bool startVal = true, float endVal = true, EasingFunction easing = EasingCallbacks::linear);

        void Reset();
        void Finish();

        void OnUpdate(Horizon::HEntityInterface* entity) override;
        void OnFixUpdate(Horizon::HEntityInterface* entity) override;

    private:
        void ApplyStateToEntity(Horizon::HEntityInterface* entity);
        float GetCurrentTime() const;

        SpriteTransformState state;
    };


}
