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

#include "Animation/Core/TransformAnimation.h"
#include <NNCore/Include/Math/HMathHelper.h>
#include "Scene/Components.h"

namespace NN::Runtime
{
	TransformAnimationState::TransformAnimationState()
	{
		Reset();
	}

	void TransformAnimationState::SetAll(const AnimationData& data)
	{
		xoffset.currentValue = data.xoffset;
		yoffset.currentValue = data.yoffset;
		rotate.currentValue = data.rotate;
		zoom.currentValue = data.zoom;
		xzoom.currentValue = data.xzoom;
		yzoom.currentValue = data.yzoom;
		visible.currentValue = data.visible ? 1.0f : 0.0f;
	}

	void TransformAnimationState::SetAll(const TransformAnimationState& data)
	{
		xoffset.currentValue = data.xoffset.currentValue;
		yoffset.currentValue = data.yoffset.currentValue;
		rotate.currentValue = data.rotate.currentValue;
		zoom.currentValue = data.zoom.currentValue;
		xzoom.currentValue = data.xzoom.currentValue;
		yzoom.currentValue = data.yzoom.currentValue;
		visible.currentValue = data.visible.currentValue;
	}

	TransformAnimationState::AnimationData TransformAnimationState::GetCurrent() const
	{
		AnimationData data;
		data.xoffset = xoffset.currentValue;
		data.yoffset = yoffset.currentValue;
		data.rotate = rotate.currentValue;
		data.zoom = zoom.currentValue;
		data.xzoom = xzoom.currentValue;
		data.yzoom = yzoom.currentValue;
		data.visible = visible.currentValue > 0.5f;
		return data;
	}

	void TransformAnimationState::Finish()
	{
		TravelProperty([this](FloatAnimationProperty& property)
			{
				property.Finish();
			});
	}

	bool TransformAnimationState::IsFinish()
	{
		return xoffset.IsFinish() &&
			yoffset.IsFinish() &&
			rotate.IsFinish() &&
			zoom.IsFinish() &&
			xzoom.IsFinish() &&
			yzoom.IsFinish() &&
			visible.IsFinish();
	}

	void TransformAnimationState::Reset()
	{
		SetAll(AnimationData());
		TravelProperty([this](FloatAnimationProperty& property)
			{
				property.active = false;
				property.tween = Tween{};
			});
	}

	void TransformAnimationState::TravelProperty(std::function<void(FloatAnimationProperty& property)> callback)
	{
		callback(xoffset);
		callback(yoffset);
		callback(rotate);
		callback(zoom);
		callback(xzoom);
		callback(yzoom);
		callback(visible);
	}

	void TransformAnimationScript::TransformXOffset(float startTime, float duration, float startVal, float endVal,
		EasingFunction easing)
	{
		state.xoffset.Start(startVal, endVal,duration, Tween{}, startTime);
	}

	void TransformAnimationScript::TransformYOffset(float startTime, float duration, float startVal, float endVal,
		EasingFunction easing)
	{
		state.yoffset.Start(startVal, endVal,duration, Tween{}, startTime);
	}

	void TransformAnimationScript::TransformRotate(float startTime, float duration, float startVal, float endVal,
		EasingFunction easing)
	{
		state.rotate.Start(startVal, endVal,duration, Tween{}, startTime);
	}

	void TransformAnimationScript::TransformZoom(float startTime, float duration, float startVal, float endVal,
		EasingFunction easing)
	{
		state.zoom.Start(startVal, endVal,duration, Tween{}, startTime);
	}

	void TransformAnimationScript::TransformXZoom(float startTime, float duration, float startVal, float endVal,
		EasingFunction easing)
	{
		state.xzoom.Start(startVal, endVal,duration, Tween{}, startTime);
	}

	void TransformAnimationScript::TransformYZoom(float startTime, float duration, float startVal, float endVal,
		EasingFunction easing)
	{
		state.yzoom.Start(startVal, endVal,duration, Tween{}, startTime);
	}

	void TransformAnimationScript::TransformVisible(float startTime, float duration, bool startVal, float endVal,
		EasingFunction easing)
	{
		state.visible.Start(startVal ? 1.0f : 0.0f, endVal ? 1.0f : 0.0f,duration, Tween{}, startTime);
	}

	void TransformAnimationScript::Reset()
	{
		state.Reset();
	}

	void TransformAnimationScript::Finish()
	{
		state.Finish();
	}

	void TransformAnimationScript::OnUpdate(NN::Core::HEntityInterface* entity)
	{
		// 更新所有属性
		state.TravelProperty([this](FloatAnimationProperty& property)
			{
				property.Update();
			});
		//state.alpha.Update(currentTime);
		//state.xoffset.Update(currentTime);
		//state.yoffset.Update(currentTime);
		//state.rotate.Update(currentTime);
		//state.zoom.Update(currentTime);
		//state.xzoom.Update(currentTime);
		//state.yzoom.Update(currentTime);
		//state.visible.Update(currentTime);

		ApplyStateToEntity(entity);
	}

	void TransformAnimationScript::OnFixUpdate(NN::Core::HEntityInterface* entity)
	{

	}

	float TransformAnimationScript::GetCurrentTime() const
	{
		return RuntimeCore::GetCurrentTime();
	}

	void TransformAnimationScript::ApplyStateToEntity(NN::Core::HEntityInterface* entity)
	{
		auto* transform = entity->GetComponent<TransformComponent>();
		if (transform)
		{
			if (state.xoffset.active)
			{
				transform->location.x = state.xoffset.currentValue;
				transform->is_dirty = true;
				//std::cout << state.xoffset.currentValue << std::endl;
			}

			if (state.yoffset.active)
			{
				transform->location.y = state.yoffset.currentValue;
				transform->is_dirty = true;
			}

			if (state.zoom.active)
			{
				transform->scale.x = state.zoom.currentValue;
				transform->scale.y = state.zoom.currentValue;
				transform->is_dirty = true;
			}

			if (state.xzoom.active)
			{
				transform->scale.x = state.xzoom.currentValue;
				transform->is_dirty = true;
			}

			if (state.yzoom.active)
			{
				transform->scale.y = state.yzoom.currentValue;
				transform->is_dirty = true;
			}
		}

	}
}
