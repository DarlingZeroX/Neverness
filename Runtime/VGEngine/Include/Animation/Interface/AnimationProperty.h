#pragma once
#include "../../Interface/GameInterface.h"

namespace VisionGal
{
    // 缓动函数类型
    using EasingFunction = std::function<float(float)>;

    // 单个属性的插值器
    struct SingleAnimationProperty {
        float startValue = 0.0f;
        float endValue = 0.0f;
        float currentValue = 0.0f;
        float startTime = 0.0f;
        float duration = 1.0f;
        EasingFunction easing = [](float t) { return t; }; // 默认线性插值
        bool active = false;
        bool isFinish = false;

        // 更新属性值
        void Update(float currentTime);
		 
        // 开始插值
        void Start(float startTime, float duration, float startValue, float endValue, EasingFunction easing = [](float t) { return t; });

        void Finish();
        bool IsFinish();
    };

    struct EasingCallbacks
    {
        static EasingFunction linear;
    };
}
