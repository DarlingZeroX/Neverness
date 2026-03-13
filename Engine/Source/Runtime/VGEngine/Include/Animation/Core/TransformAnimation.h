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
	struct TransformAnimationState {
		TransformAnimationState();

		struct AnimationData
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

		FloatAnimationProperty xoffset;
		FloatAnimationProperty yoffset;
		FloatAnimationProperty rotate;
		FloatAnimationProperty zoom;
		FloatAnimationProperty xzoom;
		FloatAnimationProperty yzoom;
		FloatAnimationProperty visible; // 将bool转为float(0.0/1.0)

		// 立即设置所有属性值
		void SetAll(const AnimationData& data);
		void SetAll(const TransformAnimationState& data);

		void Finish();
		bool IsFinish();
		void Reset();

		void TravelProperty(std::function<void(FloatAnimationProperty& property)> callback);

		// 获取当前状态（转换为原始格式）
		AnimationData GetCurrent() const;
	};

	class TransformAnimationScript : public IAnimationScript
	{
	public:
		TransformAnimationScript() = default;
		~TransformAnimationScript() override = default;
		TransformAnimationScript(const TransformAnimationScript&) = delete;
		TransformAnimationScript& operator=(const TransformAnimationScript&) = delete;
		TransformAnimationScript(TransformAnimationScript&&) noexcept = default;
		TransformAnimationScript& operator=(TransformAnimationScript&&) noexcept = default;

		// 为单个属性设置动画
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

		TransformAnimationState state;

	};


}
