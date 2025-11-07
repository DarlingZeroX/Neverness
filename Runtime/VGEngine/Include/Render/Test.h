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

#include <memory>
#include <unordered_map>
#include <functional>

namespace VisionGal::Test
{

    // 缓动函数类型
    using EasingFunction = std::function<float(float)>;

    // 单个属性的插值器
    struct AnimatedProperty {
        float startValue = 0.0f;
        float endValue = 0.0f;
        float currentValue = 0.0f;
        float startTime = 0.0f;
        float duration = 1.0f;
        EasingFunction easing = [](float t) { return t; }; // 默认线性插值
        bool active = false;

        // 更新属性值
        void Update(float currentTime) {
            if (!active || currentTime < startTime) return;

            float elapsed = currentTime - startTime;
            if (elapsed >= duration) {
                currentValue = endValue;
                active = false;
            }
            else {
                float t = elapsed / duration;
                currentValue = startValue + easing(t) * (endValue - startValue);
            }
        }
		 
        // 开始插值
        void Start(float startTime, float duration, float startValue, float endValue,
            EasingFunction easing = [](float t) { return t; }) {
            this->startTime = startTime;
            this->duration = duration;
            this->startValue = startValue;
            this->endValue = endValue;
            this->easing = easing;
            this->active = true;
        }
    };

    struct ImageTransformState {
        AnimatedProperty alpha;
        AnimatedProperty xoffset;
        AnimatedProperty yoffset; // 修正拼写错误
        AnimatedProperty rotate;
        AnimatedProperty zoom;
        AnimatedProperty xzoom;
        AnimatedProperty yzoom;
        AnimatedProperty visible; // 将bool转为float(0.0/1.0)

        // 立即设置所有属性值
        void SetAll(float a, float xoff, float yoff, float rot, float z, float xz, float yz, bool vis) {
            alpha.currentValue = a;
            xoffset.currentValue = xoff;
            yoffset.currentValue = yoff;
            rotate.currentValue = rot;
            zoom.currentValue = z;
            xzoom.currentValue = xz;
            yzoom.currentValue = yz;
            visible.currentValue = vis ? 1.0f : 0.0f;
        }

        // 获取当前状态（转换为原始格式）
        void GetCurrent(float& a, float& xoff, float& yoff, float& rot, float& z, float& xz, float& yz, bool& vis) const {
            a = alpha.currentValue;
            xoff = xoffset.currentValue;
            yoff = yoffset.currentValue;
            rot = rotate.currentValue;
            z = zoom.currentValue;
            xz = xzoom.currentValue;
            yz = yzoom.currentValue;
            vis = visible.currentValue > 0.5f;
        }
    };

    class ImageTransformScript {
    public:
        ImageTransformScript() = default;
        ~ImageTransformScript() = default;
        ImageTransformScript(const ImageTransformScript&) = delete;
        ImageTransformScript& operator=(const ImageTransformScript&) = delete;
        ImageTransformScript(ImageTransformScript&&) noexcept = default;
        ImageTransformScript& operator=(ImageTransformScript&&) noexcept = default;

        // 为单个属性设置动画
        void AnimateAlpha(float startTime, float duration, float startVal, float endVal,
            EasingFunction easing = [](float t) { return t; }) {
            state.alpha.Start(startTime, duration, startVal, endVal, easing);
        }

        // 类似地为其他属性添加AnimateXXX方法...
        void AnimateXOffset(float startTime, float duration, float startVal, float endVal,
            EasingFunction easing = [](float t) { return t; }) {
            state.xoffset.Start(startTime, duration, startVal, endVal, easing);
        }

        // 其他属性的Animate方法...

        void Start(float time) {
            // 初始化所有属性的起始值
            // 可以从startState获取初始值
            state.SetAll(
                startState.alpha, startState.xoffset, startState.yoffest,
                startState.rotate, startState.zoom, startState.xzoom,
                startState.yzoom, startState.visible
            );
        }

        void Finish() {
            // 立即应用所有属性的最终值
            state.SetAll(
                endState.alpha, endState.xoffset, endState.yoffest,
                endState.rotate, endState.zoom, endState.xzoom,
                endState.yzoom, endState.visible
            );
        }

        float GetProgress() const noexcept {
            // 计算所有动画的平均进度
            // 实际实现可能需要更复杂的逻辑
            return 0.0f;
        }

        void OnUpdate(Horizon::HEntityInterface* entity) {
            float currentTime = GetCurrentTime(); // 假设存在获取当前时间的函数

            // 更新所有属性
            state.alpha.Update(currentTime);
            state.xoffset.Update(currentTime);
            state.yoffset.Update(currentTime);
            state.rotate.Update(currentTime);
            state.zoom.Update(currentTime);
            state.xzoom.Update(currentTime);
            state.yzoom.Update(currentTime);
            state.visible.Update(currentTime);

            // 应用更新后的状态到实体
            ApplyStateToEntity(entity);
        }

        void OnFixUpdate(Horizon::HEntityInterface* entity) {
            // 固定时间步长的更新逻辑
        }

    private:
        ImageTransformState state;
        ImageTransformState startState;
        ImageTransformState endState;
        Ref<TransitionHelper> m_Helper;

        // 将当前状态应用到实体
        void ApplyStateToEntity(Horizon::HEntityInterface* entity) {
            float a, xoff, yoff, rot, z, xz, yz;
            bool vis;
            state.GetCurrent(a, xoff, yoff, rot, z, xz, yz, vis);

            // 使用获取的值设置实体属性
            entity->SetAlpha(a);
            entity->SetPosition(xoff, yoff);
            entity->SetRotation(rot);
            entity->SetScale(z * xz, z * yz);
            entity->SetVisible(vis);
        }

        // 获取当前时间的函数（需要根据实际情况实现）
        float GetCurrentTime() const {
            return 0.0f; // 实际实现中应返回游戏时间
        }
    };
}