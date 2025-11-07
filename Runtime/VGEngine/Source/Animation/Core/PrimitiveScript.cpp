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

	TranslateXAnimationScript::TranslateXAnimationScript(Horizon::HEntityInterface* entity)
		:m_Entity(entity)
	{
	}

	bool TranslateXAnimationScript::StartAnimation(float targetValue, float duration, Tween tween, bool reverse)
	{
		auto* transform = m_Entity->GetComponent<TransformComponent>();
		float startX = transform->location.x;
		float endX = startX + targetValue;

		if (reverse)
		{
			endX = startX - targetValue;
			tween.reverse();
		}

		Start(duration, startX, endX, tween);
		return true;
	}

	void TranslateXAnimationScript::OnUpdate(Horizon::HEntityInterface* entity)
	{
		UpdateProperty();

		auto* transform = entity->GetComponent<TransformComponent>();
		if (transform)
		{
			if (property.active)
			{
				transform->location.x = property.GetCurrentValue();
				transform->is_dirty = true;
				std::cout << " TranslateX: " << property.GetCurrentValue() << std::endl;
			}
		}
	}

	bool TranslateXAnimationScript::CanParse(const std::string& key) const
	{
		return key == "位置偏移X";
	}

	bool TranslateXAnimationScript::ParseLua(sol::object value, Animation2DPrimitive& primitive) const
	{
		if (value.is<double>()) {
			primitive.type = Animation2DPrimitiveType::TranslateX;
			primitive.valueF = value.as<double>();
			return true;
		}
		return false;
	}

	Animation2DPrimitiveType TranslateXAnimationScript::GetPrimitiveType()
	{
		return Animation2DPrimitiveType::TranslateX;
	}

	Ref<IAnimationScript> TranslateXAnimationScript::GetAnimationScript(
		Horizon::HEntityInterface* entity, 
		const Animation2DProperty& targetProperty,
		const Animation2DPrimitive& primitive,
		bool reverse
	)
	{
		H_ASSERT_NOT_NULL(entity);

		auto script = CreateRef<TranslateXAnimationScript>(entity);
		script->StartAnimation(primitive.valueF, targetProperty.duration, targetProperty.tween, reverse);

		return script;
	}

	TranslateYAnimationScript::TranslateYAnimationScript(Horizon::HEntityInterface* entity)
		:m_Entity(entity)
	{
	}

	bool TranslateYAnimationScript::StartAnimation(float targetValue, float duration, Tween tween, bool reverse)
	{
		auto* transform = m_Entity->GetComponent<TransformComponent>();
		float startY = transform->location.y;
		float endY = startY + targetValue;

		if (reverse)
		{
			endY = startY - targetValue;
			tween.reverse();
		}

		Start(duration, startY, endY, tween);
		return true;
	}

	void TranslateYAnimationScript::OnUpdate(Horizon::HEntityInterface* entity)
	{
		UpdateProperty();

		auto* transform = entity->GetComponent<TransformComponent>();
		if (transform)
		{
			if (property.active)
			{
				transform->location.y = property.GetCurrentValue();
				transform->is_dirty = true;
				std::cout << " TranslateY: " << property.GetCurrentValue() << std::endl;
			}
		}
	}

	bool TranslateYAnimationScript::CanParse(const std::string& key) const
	{
		return key == "位置偏移Y";
	}

	bool TranslateYAnimationScript::ParseLua(sol::object value, Animation2DPrimitive& primitive) const
	{
		if (value.is<double>()) {
			primitive.type = Animation2DPrimitiveType::TranslateY;
			primitive.valueF = value.as<double>();
			return true;
		}
		return false;
	}

	Animation2DPrimitiveType TranslateYAnimationScript::GetPrimitiveType()
	{
		return Animation2DPrimitiveType::TranslateY;
	}

	Ref<IAnimationScript> TranslateYAnimationScript::GetAnimationScript(
		Horizon::HEntityInterface* entity, 
		const Animation2DProperty& targetProperty,
		const Animation2DPrimitive& primitive,
		bool reverse
	)
	{
		H_ASSERT_NOT_NULL(entity);

		auto script = CreateRef<TranslateYAnimationScript>(entity);
		script->StartAnimation(primitive.valueF, targetProperty.duration, targetProperty.tween, reverse);

		return script;
	}

	AnimationPrimitiveManager::AnimationPrimitiveManager()
	{
		RegisterParser<TranslateXAnimationScript>();
		RegisterParser<TranslateYAnimationScript>();
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

	Ref<IAnimationScript> AnimationPrimitiveManager::GetPrimitiveScript(
		Horizon::HEntityInterface* entity,
		const Animation2DProperty& targetProperty,
		const Animation2DPrimitive& primitive,
		bool reverse
	)
	{
		H_ASSERT_NOT_NULL(entity);

		for (const auto& parser : m_Primitives) {
			if (parser->GetPrimitiveType() == primitive.type) {
				return parser->GetAnimationScript(entity, targetProperty, primitive, reverse);
			}
		}

		return nullptr;
	}
}
