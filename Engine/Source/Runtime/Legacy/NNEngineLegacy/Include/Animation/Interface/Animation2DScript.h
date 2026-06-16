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
#include "../../EngineConfig.h"
#include "../Core/AnimationCore.h"
#include <sol/table.hpp>

namespace NN::Runtime
{
	class VG_ENGINE_API Animation2DScript : public IAnimationScript
	{
	public:
		Animation2DScript(NN::Core::HEntityInterface* entity);
		~Animation2DScript() override = default;
		Animation2DScript(const Animation2DScript&) = delete;
		Animation2DScript& operator=(const Animation2DScript&) = delete;
		Animation2DScript(Animation2DScript&&) noexcept = default;
		Animation2DScript& operator=(Animation2DScript&&) noexcept = default;

		void OnUpdate(NN::Core::HEntityInterface* entity) override;

		// 开始动画
		bool Animate(const Animation2DProperty& targetProperty, int numIterations = 1, bool alternateDirection = true, float delay = 0.0f);
		bool AnimateLua(const sol::table& targetValue, float duration, std::string tween, int numIterations = 1, bool alternateDirection = true, float delay = 0.0f);

		// 添加动画关键帧
		bool AddAnimationKey(const Animation2DProperty& targetProperty);
		Animation2DScript* AddAnimationKeyLua(const sol::table& targetValue, float duration, std::string tween);

		// 解析动画属性
		static bool ParseAnimationProperty(const sol::table& value, Animation2DProperty& outProperty);
	private:
		void SetEntity(NN::Core::HEntityInterface* entity);

		// 应用动画关键帧
		void ApplyAnimationKey(Animation2DProperty& targetProperty);

		// 恢复初始状态,仅在动画完成一轮时调用
		void RestoreInitialState();
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
		std::vector<Animation2DProperty> m_AnimationKeys;
		int m_NumIterations = 1;
		bool m_AlternateDirection = true;
		NN::Core::HEntityInterface* m_Entity = nullptr;

		int m_CurrentIteration = 0;
		int m_CurrentDirection = 1;
		int m_CurrentKeyIndex = 0;
	};
}
