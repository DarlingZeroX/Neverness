#pragma once
#include "AnimationCore.h"
#include "AnimationProperty.h"

namespace VisionGal
{
	struct IAnimationPrimitive
	{
		virtual ~IAnimationPrimitive() = default;

		virtual bool CanParse(const std::string& key) const = 0;
		virtual bool ParseLua(sol::object value, Animation2DPrimitive& primitive) const = 0;
		virtual Animation2DPrimitiveType GetPrimitiveType() = 0;
		virtual Ref<IAnimationScript> GetAnimationScript(Horizon::HEntityInterface* entity, const Animation2DProperty& targetProperty, const Animation2DPrimitive& primitive, bool reverse) = 0;
	};

	struct TranslateXAnimationScript : public FloatAnimationPropertyScript, public IAnimationPrimitive
	{
	public:
		TranslateXAnimationScript() = default;
		TranslateXAnimationScript(Horizon::HEntityInterface* entity);
		TranslateXAnimationScript(const TranslateXAnimationScript&) = default;
		TranslateXAnimationScript& operator=(const TranslateXAnimationScript&) = default;
		TranslateXAnimationScript(TranslateXAnimationScript&&) noexcept = default;
		TranslateXAnimationScript& operator=(TranslateXAnimationScript&&) noexcept = default;
		~TranslateXAnimationScript() override = default;

		bool StartAnimation(float targetValue, float duration, Tween tween, bool reverse);

		void OnUpdate(Horizon::HEntityInterface* entity) override;

		bool CanParse(const std::string& key) const override;
		bool ParseLua(sol::object value, Animation2DPrimitive& primitive) const override;
		Animation2DPrimitiveType GetPrimitiveType() override;
		Ref<IAnimationScript> GetAnimationScript(Horizon::HEntityInterface* entity, const Animation2DProperty& targetProperty, const Animation2DPrimitive& primitive, bool reverse) override;
	private:
		Horizon::HEntityInterface* m_Entity;
	};

	struct TranslateYAnimationScript : public FloatAnimationPropertyScript, public IAnimationPrimitive
	{
	public:
		TranslateYAnimationScript() = default;
		TranslateYAnimationScript(Horizon::HEntityInterface* entity);
		TranslateYAnimationScript(const TranslateYAnimationScript&) = default;
		TranslateYAnimationScript& operator=(const TranslateYAnimationScript&) = default;
		TranslateYAnimationScript(TranslateYAnimationScript&&) noexcept = default;
		TranslateYAnimationScript& operator=(TranslateYAnimationScript&&) noexcept = default;
		~TranslateYAnimationScript() override = default;

		bool StartAnimation(float targetValue, float duration, Tween tween, bool reverse);

		void OnUpdate(Horizon::HEntityInterface* entity) override;

		bool CanParse(const std::string& key) const override;
		bool ParseLua(sol::object value, Animation2DPrimitive& primitive) const override;
		Animation2DPrimitiveType GetPrimitiveType() override;
		Ref<IAnimationScript> GetAnimationScript(Horizon::HEntityInterface* entity, const Animation2DProperty& targetProperty, const Animation2DPrimitive& primitive, bool reverse) override;
	private:
		Horizon::HEntityInterface* m_Entity;
	};

	// 策略管理器
	class AnimationPrimitiveManager {
	private:
		std::vector<std::unique_ptr<IAnimationPrimitive>> m_Primitives;

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

		Ref<IAnimationScript> GetPrimitiveScript(Horizon::HEntityInterface* entity, const Animation2DProperty& targetProperty, const Animation2DPrimitive& primitive, bool reverse);
	};
}
