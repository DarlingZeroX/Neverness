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

#include "Animation/Interface/Animation2DScript.h"
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

		//std::cout << "Animation2DScript::OnUpdate 当前关键帧索引: " << m_CurrentKeyIndex << std::endl;

		// 检查当前动画是否完成
		if (m_CurrentAnimationScript.IsFinished())
		{
			m_CurrentAnimationScript.Reset();

			const int keyCount = static_cast<int>(m_AnimationKeys.size());
			if (keyCount == 0)
				return;

			int nextIndex = m_CurrentKeyIndex + m_CurrentDirection;

			// 如果下一个索引仍然在范围内，直接切换到下一个关键帧
			if (nextIndex >= 0 && nextIndex < keyCount)
			{
				m_CurrentKeyIndex = nextIndex;
				ApplyAnimationKey(m_AnimationKeys[m_CurrentKeyIndex]);
				return;
			}

			// 到达边界，完成一次序列遍历
			m_CurrentIteration++;

			// 如果达到或超过迭代次数，停止继续播放
			if (m_CurrentIteration >= m_NumIterations)
			{
				// 完成所有迭代，不再应用新的关键帧
				return;
			}

			// 只有一个关键帧时，重复播放该帧
			//if (keyCount == 1)
			//{
			//	m_CurrentKeyIndex = 0;
			//	ApplyAnimationKey(m_AnimationKeys[0]);
			//	return;
			//}

			// 处理方向交替（往返）或重头开始
			if (m_AlternateDirection)
			{
				// 反转方向并移动到下一个有效索引
				m_CurrentDirection = -m_CurrentDirection;
				nextIndex = m_CurrentKeyIndex;// +m_CurrentDirection;
				if (nextIndex >= 0 && nextIndex < keyCount)
				{
					m_CurrentKeyIndex = nextIndex;
					ApplyAnimationKey(m_AnimationKeys[m_CurrentKeyIndex]);
				}
			}
			else
			{
				RestoreInitialState();
				// 不交替方向，则从头开始（正向）
				m_CurrentDirection = 1;
				m_CurrentKeyIndex = 0;
				ApplyAnimationKey(m_AnimationKeys[0]);
			}
		}
	}

	void Animation2DScript::SetEntity(Horizon::HEntityInterface* entity)
	{
		m_Entity = entity;
	}

	bool Animation2DScript::Animate(const Animation2DProperty& targetProperty, int numIterations, bool alternateDirection, float delay)
	{
		// 保证至少一次迭代（如果需要支持无限，可改为特殊值）
		m_NumIterations = (numIterations > 0) ? numIterations : 1;
		m_AlternateDirection = alternateDirection;

		// 重置运行状态，从头开始（默认正向）
		m_CurrentIteration = 0;
		m_CurrentDirection = 1;

		AddAnimationKey(targetProperty);

		// 如果是瞬时完成的情况
		while (targetProperty.duration == 0.f && m_CurrentIteration < m_NumIterations && m_CurrentIteration < 100)
		{
			OnUpdate(m_Entity);
		}
		return true;
	}

	bool Animation2DScript::AnimateLua(const sol::table& targetValue, float duration, std::string tweenStr, int numIterations,
		bool alternateDirection, float delay)
	{
		Animation2DProperty property;
		if (ParseAnimationProperty(targetValue, property))
		{
			property.duration = duration;
			if (Tween::ParseTweenByString(tweenStr, property.tween))
			{
				return Animate(property, numIterations, alternateDirection, delay);
			}
			//property.tween = Tween{};
			//return Animate(property, numIterations, alternateDirection, delay);
		}

		return false;
	}

	bool Animation2DScript::AddAnimationKey(const Animation2DProperty& targetProperty)
	{
		m_AnimationKeys.push_back(targetProperty);

		// 如果当前没有运行中的动画，则立即应用新加入的关键帧，并确保索引指向该关键帧
		if (m_CurrentAnimationScript.IsEmpty())
		{
			m_CurrentKeyIndex = static_cast<int>(m_AnimationKeys.size()) - 1;
			ApplyAnimationKey(m_AnimationKeys.back());		// 这里要用back()，因为刚加入的关键帧在最后，而不是用targetProperty临时对象
		}

		return true;
	}

	Animation2DScript* Animation2DScript::AddAnimationKeyLua(const sol::table& targetValue, float duration, std::string tweenStr)
	{
		Animation2DProperty property;
		if (ParseAnimationProperty(targetValue, property))
		{
			property.duration = duration;
			if (Tween::ParseTweenByString(tweenStr, property.tween))
			{
				this->AddAnimationKey(property);
				return this;
			}
		}

		return nullptr;
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

	void Animation2DScript::ApplyAnimationKey(Animation2DProperty& targetProperty)
	{
		auto* apm = AnimationPrimitiveManager::GetInstance();

		for (auto& primitive: targetProperty.primitive)
		{
			//AddAnimationPrimitiveScript(targetProperty, primitive);
			if (auto script = apm->StartAnimationScript(m_Entity, targetProperty, primitive, m_CurrentDirection < 0))
			{
				m_CurrentAnimationScript.scripts.push_back(script);
			}
		}

	}

	void Animation2DScript::RestoreInitialState()
	{
		auto* apm = AnimationPrimitiveManager::GetInstance();

		for (int i = m_AnimationKeys.size() - 1; i >= 0; --i)
		{
			auto key = m_AnimationKeys[i];
			key.duration = 0.f; // 设置为瞬时恢复

			for (auto& primitive : key.primitive)
			{
				if (auto script = apm->StartAnimationScript(m_Entity, key, primitive, true))
				{
					script->OnUpdate(m_Entity);
				}
			}
		}
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


