#pragma once
#include "../Core/AnimationCore.h"
#include "../../Lua/sol2/sol.hpp"

namespace VisionGal
{
	class Animation2DScript : public IAnimationScript
	{
	public:
		Animation2DScript(Horizon::HEntityInterface* entity);
		~Animation2DScript() override = default;
		Animation2DScript(const Animation2DScript&) = delete;
		Animation2DScript& operator=(const Animation2DScript&) = delete;
		Animation2DScript(Animation2DScript&&) noexcept = default;
		Animation2DScript& operator=(Animation2DScript&&) noexcept = default;

		void OnUpdate(Horizon::HEntityInterface* entity) override;

		// 开始动画
		bool Animate(const Animation2DProperty& targetProperty, int numIterations = 1, bool alternateDirection = true, float delay = 0.0f);
		bool AnimateLua(const sol::table& targetValue, float duration, std::string tween, int numIterations = 1, bool alternateDirection = true, float delay = 0.0f);

		// 添加动画关键帧
		bool AddAnimationKey(const Animation2DProperty& targetProperty);
		Animation2DScript* AddAnimationKeyLua(const sol::table& targetValue, float duration, std::string tween);

		// 解析动画属性
		static bool ParseAnimationProperty(const sol::table& value, Animation2DProperty& outProperty);
	private:
		void SetEntity(Horizon::HEntityInterface* entity);

		void ApplyAnimationKey(const Animation2DProperty& targetProperty);

		void AddAnimationPrimitiveScript(const Animation2DProperty& targetProperty,const Animation2DPrimitive& primitive);
	private:
		struct PropertyAnimationScriptList
		{
			std::vector<Ref<IAnimationScript>> scripts;

			bool IsEmpty();
			bool IsFinished();
			void Reset();
		};
	private:
		PropertyAnimationScriptList m_CurrentAnimationScript;
		std::deque<Animation2DProperty> m_AnimationKeyDeque;
		int m_NumIterations = 1;
		bool m_AlternateDirection = true;
		Horizon::HEntityInterface* m_Entity = nullptr;
	};
}
