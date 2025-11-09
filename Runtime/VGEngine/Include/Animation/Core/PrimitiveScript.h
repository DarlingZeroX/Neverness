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
#include "PrimitiveScriptInterface.h"

namespace VisionGal
{
	/// 具体策略实现
	/// 位置X轴平移动画脚本
	struct TranslateXAnimationScript : public TFloatAnimationPrimitiveScript<TranslateXAnimationScript>
	{
	public:
		TranslateXAnimationScript() = default;
		TranslateXAnimationScript(const TranslateXAnimationScript&) = default;
		TranslateXAnimationScript& operator=(const TranslateXAnimationScript&) = default;
		TranslateXAnimationScript(TranslateXAnimationScript&&) noexcept = default;
		TranslateXAnimationScript& operator=(TranslateXAnimationScript&&) noexcept = default;
		~TranslateXAnimationScript() override = default;

		bool StartEntityAnimation(Horizon::HEntityInterface* entity, Animation2DPrimitive& targetValue, float duration, Tween tween, bool reverse) override;
		bool CanParse(const std::string& key) const override;
		void ApplyValueToEntity(Horizon::HEntityInterface* entity,float value) override;
		Animation2DPrimitiveType GetPrimitiveType() const override;
	};

	/// 位置Y轴平移动画脚本
	struct TranslateYAnimationScript : public TFloatAnimationPrimitiveScript<TranslateYAnimationScript>
	{
	public:
		TranslateYAnimationScript() = default;
		TranslateYAnimationScript(const TranslateYAnimationScript&) = default;
		TranslateYAnimationScript& operator=(const TranslateYAnimationScript&) = default;
		TranslateYAnimationScript(TranslateYAnimationScript&&) noexcept = default;
		TranslateYAnimationScript& operator=(TranslateYAnimationScript&&) noexcept = default;
		~TranslateYAnimationScript() override = default;

		bool StartEntityAnimation(Horizon::HEntityInterface* entity, Animation2DPrimitive& targetValue, float duration, Tween tween, bool reverse) override;
		bool CanParse(const std::string& key) const override;
		void ApplyValueToEntity(Horizon::HEntityInterface* entity,float value) override;
		Animation2DPrimitiveType GetPrimitiveType() const override;
	};

	/// 缩放X轴动画脚本
	struct ScaleXAnimationScript : public TFloatAnimationPrimitiveScript<ScaleXAnimationScript>
	{
	public:
		ScaleXAnimationScript() = default;
		ScaleXAnimationScript(const ScaleXAnimationScript&) = default;
		ScaleXAnimationScript& operator=(const ScaleXAnimationScript&) = default;
		ScaleXAnimationScript(ScaleXAnimationScript&&) noexcept = default;
		ScaleXAnimationScript& operator=(ScaleXAnimationScript&&) noexcept = default;
		~ScaleXAnimationScript() override = default;

		bool StartEntityAnimation(Horizon::HEntityInterface* entity, Animation2DPrimitive& targetValue, float duration, Tween tween, bool reverse) override;
		bool CanParse(const std::string& key) const override;
		void ApplyValueToEntity(Horizon::HEntityInterface* entity,float value) override;
		Animation2DPrimitiveType GetPrimitiveType() const override;
	};

	/// 缩放Y轴动画脚本
	struct ScaleYAnimationScript : public TFloatAnimationPrimitiveScript<ScaleYAnimationScript>
	{
	public:
		ScaleYAnimationScript() = default;
		ScaleYAnimationScript(const ScaleYAnimationScript&) = default;
		ScaleYAnimationScript& operator=(const ScaleYAnimationScript&) = default;
		ScaleYAnimationScript(ScaleYAnimationScript&&) noexcept = default;
		ScaleYAnimationScript& operator=(ScaleYAnimationScript&&) noexcept = default;
		~ScaleYAnimationScript() override = default;

		bool StartEntityAnimation(Horizon::HEntityInterface* entity, Animation2DPrimitive& targetValue, float duration, Tween tween, bool reverse) override;
		bool CanParse(const std::string& key) const override;
		void ApplyValueToEntity(Horizon::HEntityInterface* entity, float value) override;
		Animation2DPrimitiveType GetPrimitiveType() const override;
	};

	/// 缩放动画脚本
	struct ScaleAnimationScript : public TFloat2AnimationPrimitiveScript<ScaleAnimationScript>
	{
	public:
		ScaleAnimationScript() = default;
		ScaleAnimationScript(const ScaleAnimationScript&) = default;
		ScaleAnimationScript& operator=(const ScaleAnimationScript&) = default;
		ScaleAnimationScript(ScaleAnimationScript&&) noexcept = default;
		ScaleAnimationScript& operator=(ScaleAnimationScript&&) noexcept = default;
		~ScaleAnimationScript() override = default;

		bool StartEntityAnimation(Horizon::HEntityInterface* entity, Animation2DPrimitive& targetValue, float duration, Tween tween, bool reverse) override;
		bool CanParse(const std::string& key) const override;
		void ApplyValueToEntity(Horizon::HEntityInterface* entity, float2 value) override;
		Animation2DPrimitiveType GetPrimitiveType() const override;

		bool ParseLua(sol::object value, Animation2DPrimitive& primitive) const override;
	};

	/// 旋转轴动画脚本
	struct RotateAnimationScript : public TFloatAnimationPrimitiveScript<RotateAnimationScript>
	{
	public:
		RotateAnimationScript() = default;
		RotateAnimationScript(const RotateAnimationScript&) = default;
		RotateAnimationScript& operator=(const RotateAnimationScript&) = default;
		RotateAnimationScript(RotateAnimationScript&&) noexcept = default;
		RotateAnimationScript& operator=(RotateAnimationScript&&) noexcept = default;
		~RotateAnimationScript() override = default;

		bool StartEntityAnimation(Horizon::HEntityInterface* entity, Animation2DPrimitive& targetValue, float duration, Tween tween, bool reverse) override;
		bool CanParse(const std::string& key) const override;
		void ApplyValueToEntity(Horizon::HEntityInterface* entity, float value) override;
		Animation2DPrimitiveType GetPrimitiveType() const override;
	};

	/// 透明度动画脚本
	struct SpriteAlphaAnimationScript :public TFloatAnimationPrimitiveScript<SpriteAlphaAnimationScript>
	{
	public:
		SpriteAlphaAnimationScript() = default;
		SpriteAlphaAnimationScript(const SpriteAlphaAnimationScript&) = default;
		SpriteAlphaAnimationScript& operator=(const SpriteAlphaAnimationScript&) = default;
		SpriteAlphaAnimationScript(SpriteAlphaAnimationScript&&) noexcept = default;
		SpriteAlphaAnimationScript& operator=(SpriteAlphaAnimationScript&&) noexcept = default;
		~SpriteAlphaAnimationScript() override = default;

		bool StartEntityAnimation(Horizon::HEntityInterface* entity, Animation2DPrimitive& targetValue, float duration, Tween tween, bool reverse) override;
		bool CanParse(const std::string& key) const override;
		void ApplyValueToEntity(Horizon::HEntityInterface* entity,float value) override;
		Animation2DPrimitiveType GetPrimitiveType() const override;
	};

	struct SpriteColor3AnimationScript : public Float3AnimationPrimitiveScript
	{
	public:
		SpriteColor3AnimationScript() = default;
		SpriteColor3AnimationScript(const SpriteColor3AnimationScript&) = default;
		SpriteColor3AnimationScript& operator=(const SpriteColor3AnimationScript&) = default;
		SpriteColor3AnimationScript(SpriteColor3AnimationScript&&) noexcept = default;
		SpriteColor3AnimationScript& operator=(SpriteColor3AnimationScript&&) noexcept = default;
		~SpriteColor3AnimationScript() override = default;
	
		bool StartEntityAnimation(Horizon::HEntityInterface* entity, Animation2DPrimitive& targetValue, float duration, Tween tween, bool reverse) override;

		void ApplyValueToEntity(Horizon::HEntityInterface* entity, const float3& value) override;
		bool CanParse(const std::string& key) const override;
		bool ParseLua(sol::object value, Animation2DPrimitive& primitive) const override;
		Animation2DPrimitiveType GetPrimitiveType() const override;
		Ref<IAnimationScript> StartAnimationScript(Horizon::HEntityInterface* entity, const Animation2DProperty& targetProperty, Animation2DPrimitive& primitive, bool reverse) override;
	private:
		Horizon::HEntityInterface* m_Entity;
	};

	// 策略管理器
	class AnimationPrimitiveManager {
	private:
		std::vector<std::unique_ptr<IAnimationPrimitiveScript>> m_Primitives;

	public:
		AnimationPrimitiveManager();
		~AnimationPrimitiveManager() = default;

		static AnimationPrimitiveManager* GetInstance();

		template<typename T>
		void RegisterParser()
		{
			m_Primitives.push_back(std::make_unique<T>());
		}

		bool Parse(const std::string& key, sol::object value, Animation2DPrimitive& primitive) const;

		Ref<IAnimationScript> StartAnimationScript(Horizon::HEntityInterface* entity, const Animation2DProperty& targetProperty, Animation2DPrimitive& primitive, bool reverse);
	};
}
