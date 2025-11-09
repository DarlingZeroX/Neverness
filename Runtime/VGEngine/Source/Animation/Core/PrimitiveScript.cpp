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

#include "Animation/Core/PrimitiveScript.h"
#include "Scene/Components.h"

namespace VisionGal
{
	bool TranslateXAnimationScript::StartEntityAnimation(Horizon::HEntityInterface* entity, Animation2DPrimitive& targetValue, float duration, Tween tween, bool reverse)
	{
		auto* transform = entity->GetComponent<TransformComponent>();
		float startX = transform->location.x;
		float endX = startX + targetValue.valueF;

		return StartEntityAnimationBase(startX, endX, targetValue, duration, tween, reverse);
	}

	bool TranslateXAnimationScript::CanParse(const std::string& key) const
	{
		return key == "位置偏移X";
	}

	void TranslateXAnimationScript::ApplyValueToEntity(Horizon::HEntityInterface* entity, float value)
	{
		if (auto* transform = entity->GetComponent<TransformComponent>())
		{
			transform->location.x = value;
			transform->is_dirty = true;
		}
	}

	Animation2DPrimitiveType TranslateXAnimationScript::GetPrimitiveType() const
	{
		return Animation2DPrimitiveType::TranslateX;
	}



	bool TranslateYAnimationScript::StartEntityAnimation(Horizon::HEntityInterface* entity, Animation2DPrimitive& targetValue, float duration, Tween tween, bool reverse)
	{
		auto* transform = entity->GetComponent<TransformComponent>();
		float startY = transform->location.y;
		float endY = startY + targetValue.valueF;

		return StartEntityAnimationBase(startY, endY, targetValue, duration, tween, reverse);
	}

	bool TranslateYAnimationScript::CanParse(const std::string& key) const
	{
		return key == "位置偏移Y";
	}

	void TranslateYAnimationScript::ApplyValueToEntity(Horizon::HEntityInterface* entity, float value)
	{
		if (auto* transform = entity->GetComponent<TransformComponent>())
		{
			transform->location.y = value;
			transform->is_dirty = true;
		}
	}

	Animation2DPrimitiveType TranslateYAnimationScript::GetPrimitiveType() const
	{
		return Animation2DPrimitiveType::TranslateY;
	}



	bool ScaleXAnimationScript::StartEntityAnimation(Horizon::HEntityInterface* entity,
		Animation2DPrimitive& targetValue, float duration, Tween tween, bool reverse)
	{
		auto* transform = entity->GetComponent<TransformComponent>();
		float startX = transform->scale.x;
		float endX = targetValue.valueF;

		return StartEntityAnimationBase(startX, endX, targetValue, duration, tween, reverse);
	}

	bool ScaleXAnimationScript::CanParse(const std::string& key) const
	{
		return key == "缩放X";
	}

	void ScaleXAnimationScript::ApplyValueToEntity(Horizon::HEntityInterface* entity, float value)
	{
		if (auto* transform = entity->GetComponent<TransformComponent>())
		{
			transform->scale.x = value;
			transform->is_dirty = true;

			//std::cout << "ScaleX applied: " << value << std::endl;
		}
	}

	Animation2DPrimitiveType ScaleXAnimationScript::GetPrimitiveType() const
	{
		return Animation2DPrimitiveType::ScaleX;
	}



	bool ScaleYAnimationScript::StartEntityAnimation(Horizon::HEntityInterface* entity,
		Animation2DPrimitive& targetValue, float duration, Tween tween, bool reverse)
	{
		auto* transform = entity->GetComponent<TransformComponent>();
		float startY = transform->scale.y;
		float endY = targetValue.valueF;

		return StartEntityAnimationBase(startY, endY, targetValue, duration, tween, reverse);
	}

	bool ScaleYAnimationScript::CanParse(const std::string& key) const
	{
		return key == "缩放Y";
	}

	void ScaleYAnimationScript::ApplyValueToEntity(Horizon::HEntityInterface* entity, float value)
	{
		if (auto* transform = entity->GetComponent<TransformComponent>())
		{
			transform->scale.y = value;
			transform->is_dirty = true;
		}
	}

	Animation2DPrimitiveType ScaleYAnimationScript::GetPrimitiveType() const
	{
		return Animation2DPrimitiveType::ScaleY;
	}



	bool ScaleAnimationScript::StartEntityAnimation(Horizon::HEntityInterface* entity,
	                                                Animation2DPrimitive& targetValue, float duration, Tween tween, bool reverse)
	{
		auto* transform = entity->GetComponent<TransformComponent>();
		float startScaleX = transform->scale.x;
		float startScaleY = transform->scale.y;
		float endScaleX = targetValue.valueF;
		float endScaleY = targetValue.valueF;
		float2 startScale = float2(startScaleX, startScaleY);
		float2 endScale = float2(endScaleX, endScaleY);

		if (reverse)
		{
			endScale = targetValue.__StartValueF2;
			tween.reverse();
		}

		targetValue.__StartValueF2 = startScale;

		property.Start(startScale, endScale, duration, tween);
		return true;
	}

	bool ScaleAnimationScript::CanParse(const std::string& key) const
	{
		return key == "缩放";
	}

	void ScaleAnimationScript::ApplyValueToEntity(Horizon::HEntityInterface* entity, float2 value)
	{
		if (auto* transform = entity->GetComponent<TransformComponent>())
		{
			transform->scale.x = value.x;
			transform->scale.y = value.y;
			transform->is_dirty = true;
		}
	}

	Animation2DPrimitiveType ScaleAnimationScript::GetPrimitiveType() const
	{
		return Animation2DPrimitiveType::Scale;
	}

	bool ScaleAnimationScript::ParseLua(sol::object value, Animation2DPrimitive& primitive) const
	{
		if (value.is<double>()) {
			primitive.type = GetPrimitiveType();
			primitive.valueF = value.as<double>();
			return true;
		}
		return false;
	}



	bool RotateAnimationScript::StartEntityAnimation(Horizon::HEntityInterface* entity,
		Animation2DPrimitive& targetValue, float duration, Tween tween, bool reverse)
	{
		auto* transform = entity->GetComponent<TransformComponent>();
		float3 rotation = glm::degrees(glm::eulerAngles(transform->rotation));

		float startY = rotation.z;
		float endY = targetValue.valueF;

		return StartEntityAnimationBase(startY, endY, targetValue, duration, tween, reverse);
	}

	bool RotateAnimationScript::CanParse(const std::string& key) const
	{
		return key == "旋转";
	}

	void RotateAnimationScript::ApplyValueToEntity(Horizon::HEntityInterface* entity, float value)
	{
		if (auto* transform = entity->GetComponent<TransformComponent>())
		{
			float3 rotation = glm::degrees(glm::eulerAngles(transform->rotation));
			rotation.z = value;
			transform->rotation = quaternion( glm::radians(rotation) );
			transform->is_dirty = true;
		}
	}

	Animation2DPrimitiveType RotateAnimationScript::GetPrimitiveType() const
	{
		return Animation2DPrimitiveType::Rotate;
	}



	bool SpriteAlphaAnimationScript::StartEntityAnimation(Horizon::HEntityInterface* entity, Animation2DPrimitive& targetValue, float duration, Tween tween, bool reverse)
	{
		auto* sprite = entity->GetComponent<SpriteRendererComponent>();
		float start = sprite->color.a;
		float end = targetValue.valueF;

		return StartEntityAnimationBase(start, end, targetValue, duration, tween, reverse);
	}

	bool SpriteAlphaAnimationScript::CanParse(const std::string& key) const
	{
		return key == "精灵透明度";
	}

	void SpriteAlphaAnimationScript::ApplyValueToEntity(Horizon::HEntityInterface* entity, float value)
	{
		if (auto* sprite = entity->GetComponent<SpriteRendererComponent>())
		{
			sprite->color.a = property.GetCurrentValue();
		}
	}



	Animation2DPrimitiveType SpriteAlphaAnimationScript::GetPrimitiveType() const
	{
		return Animation2DPrimitiveType::SpriteAlpha;
	}

	bool SpriteColor3AnimationScript::StartEntityAnimation(Horizon::HEntityInterface* entity, Animation2DPrimitive& targetValue,
		float duration, Tween tween, bool reverse)
	{
		auto* sprite = entity->GetComponent<SpriteRendererComponent>();
		auto& color = sprite->color;
		float3 start = float3(color.r, color.g, color.b);
		float3 end = targetValue.valueF3;

		if (reverse)
		{
			end = targetValue.__StartValueF3;
			tween.reverse();
		}

		targetValue.__StartValueF3 = start;

		property.Start(start, end, duration, tween);
		return true;
	}

	void SpriteColor3AnimationScript::ApplyValueToEntity(Horizon::HEntityInterface* entity, const float3& value)
	{
		if (auto* sprite = entity->GetComponent<SpriteRendererComponent>())
		{
			float3 color = value;
			sprite->color.r = color.r;
			sprite->color.g = color.g;
			sprite->color.b = color.b;
		}
	}

	bool SpriteColor3AnimationScript::CanParse(const std::string& key) const
	{
		return key == "精灵颜色RGB";
	}

	bool SpriteColor3AnimationScript::ParseLua(sol::object value, Animation2DPrimitive& primitive) const
	{
		if (value.is<sol::table>()) {
			sol::table tbl = value.as<sol::table>();
			float r = tbl.get_or("r", 0.0);
			float g = tbl.get_or("g", 0.0);
			float b = tbl.get_or("b", 0.0);
			primitive.type = Animation2DPrimitiveType::SpriteColor3;
			primitive.valueF3 = float3(r, g, b);
			return true;
		}
		return false;
	}

	Animation2DPrimitiveType SpriteColor3AnimationScript::GetPrimitiveType() const
	{
		return Animation2DPrimitiveType::SpriteColor3;
	}

	Ref<IAnimationScript> SpriteColor3AnimationScript::StartAnimationScript(Horizon::HEntityInterface* entity,
		const Animation2DProperty& targetProperty, Animation2DPrimitive& primitive, bool reverse)
	{
		H_ASSERT_NOT_NULL(entity);

		auto script = CreateRef<SpriteColor3AnimationScript>();
		script->StartEntityAnimation(entity, primitive, targetProperty.duration, targetProperty.tween, reverse);

		return script;
	}

	AnimationPrimitiveManager::AnimationPrimitiveManager()
	{
		RegisterParser<TranslateXAnimationScript>();
		RegisterParser<TranslateYAnimationScript>();
		RegisterParser<ScaleXAnimationScript>();
		RegisterParser<ScaleYAnimationScript>();
		RegisterParser<ScaleAnimationScript>();
		RegisterParser<RotateAnimationScript>();
		RegisterParser<SpriteAlphaAnimationScript>();
		// 可以轻松添加新的解析器
	}

	AnimationPrimitiveManager* AnimationPrimitiveManager::GetInstance()
	{
		static AnimationPrimitiveManager instance;
		return &instance;
	}

	bool AnimationPrimitiveManager::Parse(
		const std::string& key, 
		sol::object value,
		Animation2DPrimitive& primitive
	) const
	{
		for (const auto& parser : m_Primitives) {
			if (parser->CanParse(key)) {
				return parser->ParseLua(value, primitive);
			}
		}

		return false;
	}

	Ref<IAnimationScript> AnimationPrimitiveManager::StartAnimationScript(
		Horizon::HEntityInterface* entity,
		const Animation2DProperty& targetProperty,
		Animation2DPrimitive& primitive,
		bool reverse
	)
	{
		H_ASSERT_NOT_NULL(entity);

		for (const auto& parser : m_Primitives) {
			if (parser->GetPrimitiveType() == primitive.type) {
				return parser->StartAnimationScript(entity, targetProperty,  const_cast<Animation2DPrimitive&>(primitive), reverse);
			}
		}

		return nullptr;
	}
}
