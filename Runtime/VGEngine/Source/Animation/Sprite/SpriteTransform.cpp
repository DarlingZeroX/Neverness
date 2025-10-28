#include "Animation/Sprite/SpriteTransform.h"
#include <HCore/Include/Math/HMathHelper.h>

#include "Scene/Components.h"

namespace VisionGal
{
	SpriteTransformState::SpriteTransformState()
	{
		Reset();
	}

	void SpriteTransformState::SetAll(const TransformData& data)
	{
		alpha.currentValue = data.alpha;
		xoffset.currentValue = data.xoffset;
		yoffset.currentValue = data.yoffset;
		rotate.currentValue = data.rotate;
		zoom.currentValue = data.zoom;
		xzoom.currentValue = data.xzoom;
		yzoom.currentValue = data.yzoom;
		visible.currentValue = data.visible ? 1.0f : 0.0f;
	}

	void SpriteTransformState::SetAll(const SpriteTransformState& data)
	{
		alpha.currentValue = data.alpha.currentValue;
		xoffset.currentValue = data.xoffset.currentValue;
		yoffset.currentValue = data.yoffset.currentValue;
		rotate.currentValue = data.rotate.currentValue;
		zoom.currentValue = data.zoom.currentValue;
		xzoom.currentValue = data.xzoom.currentValue;
		yzoom.currentValue = data.yzoom.currentValue;
		visible.currentValue = data.visible.currentValue;
	}

	SpriteTransformState::TransformData SpriteTransformState::GetCurrent() const
	{
		TransformData data;
		data.alpha = alpha.currentValue;
		data.xoffset = xoffset.currentValue;
		data.yoffset = yoffset.currentValue;
		data.rotate = rotate.currentValue;
		data.zoom = zoom.currentValue;
		data.xzoom = xzoom.currentValue;
		data.yzoom = yzoom.currentValue;
		data.visible = visible.currentValue > 0.5f;
		return data;
	}

	void SpriteTransformState::Finish()
	{
		TravelProperty([this](SingleAnimationProperty& property)
			{
				property.Finish();
			});
	}

	bool SpriteTransformState::IsFinish()
	{
		return alpha.IsFinish() &&
		xoffset.IsFinish() &&
		yoffset.IsFinish() &&
		rotate.IsFinish() &&
		zoom.IsFinish() &&
		xzoom.IsFinish() &&
		yzoom.IsFinish() &&
		visible.IsFinish();
	}

	void SpriteTransformState::Reset()
	{
		SetAll(TransformData());
		TravelProperty([this](SingleAnimationProperty& property)
			{
				property.active = false;
				property.easing = EasingCallbacks::linear;
			});
	}

	void SpriteTransformState::TravelProperty(std::function<void(SingleAnimationProperty& property)> callback)
	{
		callback(alpha);
		callback(xoffset);
		callback(yoffset);
		callback(rotate);
		callback(zoom);
		callback(xzoom);
		callback(yzoom);
		callback(visible);
	}

	void SpriteTransformScript::TransformAlpha(float startTime, float duration, float startVal, float endVal,
	                                           EasingFunction easing)
	{
		state.alpha.Start(startTime, duration, startVal, endVal, easing);
	}

	void SpriteTransformScript::TransformXOffset(float startTime, float duration, float startVal, float endVal,
		EasingFunction easing)
	{
		state.xoffset.Start(startTime, duration, startVal, endVal, easing);
	}

	void SpriteTransformScript::TransformYOffset(float startTime, float duration, float startVal, float endVal,
		EasingFunction easing)
	{
		state.yoffset.Start(startTime, duration, startVal, endVal, easing);
	}

	void SpriteTransformScript::TransformRotate(float startTime, float duration, float startVal, float endVal,
		EasingFunction easing)
	{
		state.rotate.Start(startTime, duration, startVal, endVal, easing);
	}

	void SpriteTransformScript::TransformZoom(float startTime, float duration, float startVal, float endVal,
		EasingFunction easing)
	{
		state.zoom.Start(startTime, duration, startVal, endVal, easing);
	}

	void SpriteTransformScript::TransformXZoom(float startTime, float duration, float startVal, float endVal,
		EasingFunction easing)
	{
		state.xzoom.Start(startTime, duration, startVal, endVal, easing);
	}

	void SpriteTransformScript::TransformYZoom(float startTime, float duration, float startVal, float endVal,
		EasingFunction easing)
	{
		state.yzoom.Start(startTime, duration, startVal, endVal, easing);
	}

	void SpriteTransformScript::TransformVisible(float startTime, float duration, bool startVal, float endVal,
		EasingFunction easing)
	{
		state.visible.Start(startTime, duration, startVal ? 1.0f : 0.0f, endVal ? 1.0f : 0.0f, easing);
	}

	void SpriteTransformScript::Reset()
	{
		state.Reset();
	}

	void SpriteTransformScript::Finish()
	{
		state.Finish();
	}

	void SpriteTransformScript::OnUpdate(Horizon::HEntityInterface* entity)
	{
		float currentTime = GetCurrentTime(); // 假设存在获取当前时间的函数

		// 更新所有属性
		state.TravelProperty([this, currentTime](SingleAnimationProperty& property)
			{
				property.Update(currentTime);
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

	void SpriteTransformScript::OnFixUpdate(Horizon::HEntityInterface* entity)
	{

	}

	float SpriteTransformScript::GetCurrentTime() const
	{
		return Core::GetCurrentTime();
	}

	void SpriteTransformScript::ApplyStateToEntity(Horizon::HEntityInterface* entity)
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

		auto* sr = entity->GetComponent<SpriteRendererComponent>();
		if (sr)
		{
			if (state.alpha.active)
			{
				sr->color.a = state.alpha.currentValue;
				//H_LOG_INFO("sr->color.a = %f", sr->color.a);
			}
		}
	}
}
