#include "Animation/Core/Animation2DScript.h"
#include "Animation/Core/PrimitiveScript.h"

namespace VisionGal
{
	Animation2DScript::Animation2DScript(Horizon::HEntityInterface* entity)
	{
		SetEntity(entity);
	}

	void Animation2DScript::OnUpdate(Horizon::HEntityInterface* entity)
	{
		for (auto& script: m_CurrentAnimationScript.scripts)
		{
			script->OnUpdate(entity);
		}

		// 检查当前动画是否完成
		if (m_CurrentAnimationScript.IsFinished())
		{
			m_CurrentAnimationScript.Reset();

			// 应用下一个动画关键帧
			if (m_AnimationKeyDeque.empty() == false)
			{
				auto property = m_AnimationKeyDeque.front();
				m_AnimationKeyDeque.pop_front();

				ApplyAnimationKey(property);
			}
		}
	}

	void Animation2DScript::SetEntity(Horizon::HEntityInterface* entity)
	{
		m_Entity = entity;
	}

	bool Animation2DScript::Animate(const Animation2DProperty& targetProperty, int numIterations, bool alternateDirection, float delay)
	{
		m_NumIterations = numIterations;
		m_AlternateDirection = alternateDirection;

		AddAnimationKey(targetProperty);
		return true;
	}

	bool Animation2DScript::AnimateLua(const sol::table& targetValue, float duration, std::string tween, int numIterations,
		bool alternateDirection, float delay)
	{
		Animation2DProperty property;
		if (ParseAnimationProperty(targetValue, property))
		{
			property.duration = duration;
			property.tween = tween;

			return Animate(property, numIterations, alternateDirection, delay);
		}

		return false;
	}

	bool Animation2DScript::AddAnimationKey(const Animation2DProperty& targetProperty)
	{
		if (m_CurrentAnimationScript.IsEmpty())
		{
			ApplyAnimationKey(targetProperty);
		}
		else
		{
			m_AnimationKeyDeque.push_back(targetProperty);
		}

		return true;
	}

	Animation2DScript* Animation2DScript::AddAnimationKeyLua(const sol::table& targetValue, float duration, std::string tween)
	{
		Animation2DProperty property;
		if (ParseAnimationProperty(targetValue, property))
		{
			property.duration = duration;
			property.tween = tween;

			this->AddAnimationKey(property);
		}

		return this;
	}

	bool Animation2DScript::ParseAnimationProperty(const sol::table& table, Animation2DProperty& outProperty)
	{
		outProperty.Reset();
		//std::cout << "接收到的字典内容:" << std::endl;

		auto* apm = AnimationPrimitiveManager::GetInstance();

		table.for_each([&](sol::object key, sol::object value) {
			std::string keyStr = key.as<std::string>();
			Animation2DPrimitive primitive;

			if (apm->Parse(keyStr, value, primitive))
			{
				outProperty.primitive.push_back(primitive);
			}
		});

		if (outProperty.primitive.empty() == true)
			return false;

		return true;
	}

	void Animation2DScript::ApplyAnimationKey(const Animation2DProperty& targetProperty)
	{
		for (auto& primitive: targetProperty.primitive)
		{
			AddAnimationPrimitiveScript(targetProperty, primitive);
		}

	}

	void Animation2DScript::AddAnimationPrimitiveScript(const Animation2DProperty& targetProperty, const Animation2DPrimitive& primitive)
	{
		auto* apm = AnimationPrimitiveManager::GetInstance();

		if (auto script = apm->GetPrimitiveScript(m_Entity, targetProperty, primitive))
		{
			m_CurrentAnimationScript.scripts.push_back(script);
		}

		//switch (primitive.type)
		//{
		//case Animation2DPrimitiveType::TranslateX:
		//	{
		//	auto script = CreateRef<TranslateXAnimationScript>(m_Entity);
		//	script->StartAnimation(primitive.valueF, targetProperty.duration, EasingCallbacks::linear);
		//	m_CurrentAnimationScript.scripts.push_back(script);
		//	}
		//	break;
		//case Animation2DPrimitiveType::TranslateY:
		//	{
		//	auto script = CreateRef<TranslateYAnimationScript>(m_Entity);
		//	script->StartAnimation(primitive.valueF, targetProperty.duration, EasingCallbacks::linear);
		//	m_CurrentAnimationScript.scripts.push_back(script);
		//	}
		//	break;
		//}
	}

	bool Animation2DScript::PropertyAnimationScriptList::IsEmpty()
	{
		return scripts.empty();
	}

	bool Animation2DScript::PropertyAnimationScriptList::IsFinished()
	{
		bool isFinished = true;

		for (auto& script : scripts)
		{
			isFinished = isFinished && script->IsFinished();
			// Here we would check if each script is finished.
			// For simplicity, we assume they are always running.
		}

		return isFinished;
	}

	void Animation2DScript::PropertyAnimationScriptList::Reset()
	{
		scripts.clear();
	}
}


